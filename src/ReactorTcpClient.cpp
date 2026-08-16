#include "qinmo/net/ReactorTcpClient.h"
#include "qinmo/net/EventLoop.h"
#include "qinmo/base/Logger.h"

namespace qinmo::net
{

ReactorTcpClient::ReactorTcpClient(EventLoop* loop, const InetAddr& serverAddr)
    : loop_(loop)
    , serverAddr_(serverAddr)
    , state_(ClientState::kDisconnect)
    , isRetry_(false)
    , needStart_(true)
    , connectFunc_(detail::defaultFuncConn)
    , disconnectFunc_(detail::defaultFuncConn)
    , messageFunc_(detail::defaultFuncMessage)
    , writeCompleteFunc_(detail::defaultFuncConn)
    , channel_(nullptr)
{
    QINMO_INFO("ReactorTcpClient create.");
}

ReactorTcpClient::~ReactorTcpClient()
{
    ;
}



void ReactorTcpClient::connect()
{
    QINMO_INFO("Client begin connecting. serverIP: ", serverAddr_.getIP());

    needStart_.store(true);
    loop_->runInLoop( [this]() { start(); } );
}

void ReactorTcpClient::disconnect()
{
    needStart_.store(false);
}

void ReactorTcpClient::stop()
{
    needStart_.store(false);
}



EventLoop* ReactorTcpClient::getEventLoop()
{
    return loop_;
}

bool ReactorTcpClient::isRetry() const
{
    return isRetry_.load();
}

void ReactorTcpClient::setRetry(bool enable)
{
    isRetry_.store(enable);
}



void ReactorTcpClient::setConnectFunc(const ConnectFunc& f)
{
    connectFunc_ = f;
}

void ReactorTcpClient::setDisconnectFunc(const DisconnectFunc& f)
{
    disconnectFunc_ = f;
}

void ReactorTcpClient::setMessageFunc(const MessageFunc& f)
{
    messageFunc_ = f;
}

void ReactorTcpClient::setWriteCompleteFunc(const WriteCompleteFunc& f)
{
    writeCompleteFunc_ = f;
}



void ReactorTcpClient::removeChannel()
{
    channel_->disableAll();
    channel_->remove();
    loop_->queueInLoop( [this]() -> void { channel_.reset(); } );
}

void ReactorTcpClient::start()
{
    if (!needStart_.load())
    {
        QINMO_DEBUG("Start condition is not met.");
        return;
    }

    sock_ = SocketTCP::createNonBlockOrDie(serverAddr_);
    int saveError = 0;
    if (!sock_.connect(serverAddr_))
        saveError = errno;

    switch(saveError)
    {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
        {
            state_ = ClientState::kConnecting;
            channel_.reset(new Channel(loop_, sock_.getfd()));
            channel_->setWriteEvent([this]() { handleWrite(); });
            channel_->setErrorEvent([this]() { handleError(); });
            channel_->enableWrite();
            break;
        }

        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
        {
            retry();
            break;
        }

        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
        {
            QINMO_ERROR("connect error.");
            sock_.close();
            break;
        }

        default:
        {
            QINMO_ERROR("Unexpected error.");
            sock_.close();
            break;
        }
    }
}

void ReactorTcpClient::retry()
{
    state_ = ClientState::kDisconnect;
    sock_.close();
    if (isRetry_ && needStart_)
    {
        loop_->timerAfter(retryMs_ / 1000, [this]() -> void { start(); } );
        retryMs_ = std::min(retryMs_ * 2, kMaxRetryMs);
    }
    else
    {
        QINMO_DEBUG("Do not call retry.");
    }
}

void ReactorTcpClient::newConnect()
{
    TcpConnect sock(std::move(sock_));
    if (!sock.isValid())
    {
        QINMO_ERROR("Failed to attach - connect invalid. cfd=", sock.getfd());
        return;
    }

    InetAddr local = sock_.getLocalAddr();
    InetAddr peer = sock_.getPeerAddr();
    if (!peer.isValid())
        QINMO_ERROR("Failed to get address. local: ip = ", local.getIP(), " port = ", local.getPort(), "; peer: ip = ", peer.getIP(), " port = ", peer.getPort());

    RTcpConnPtr conn =
        std::make_shared<ReactorTcpConnection>(
            loop_,
            std::move(sock),
            local,
            peer
        );
    conn->setConnectFunc(connectFunc_);
    conn->setDisconnectFunc(disconnectFunc_);
    conn->setMessageFunc(messageFunc_);
    conn->setWriteCompleteFunc(writeCompleteFunc_);
    conn->setCloseFunc( [this]() -> void { removeConnect(); } );

    {
        std::lock_guard<std::mutex> lock(mutex_);
        connection_ = conn;
    }
    conn->connectEstablished();
}

void ReactorTcpClient::removeConnect()
{
    RTcpConnPtr conn;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        conn = connection_;
        connection_.reset();
    }

    loop_->queueInLoop( [this, conn]() -> void { conn->connectDestroyed(); } );
    if (isRetry() && needStart_.load())
    {
        ;
    }
}



void ReactorTcpClient::handleWrite()
{
    // handleError has been called
    if (ClientState::kConnecting != state_)
        return;

    // Tcp connection established
    removeChannel();
    int error = detail::getSocketError(sock_.getfd());
    if (error)
    {
        QINMO_WARN("Connect error. fd = ", sock_.getfd(), " errorCode = ", error);
        retry();
        return;
    }
    else if (detail::isConnectSelf(sock_.getfd()))
    {
        QINMO_WARN("Connect connect-self. fd = ", sock_.getfd());
        retry();
        return;
    }

    if (!needStart_.load())
    {
        state_ = ClientState::kDisconnect;
        sock_.close();
    }
    else
    {
        state_ = ClientState::kConnected;
        newConnect();
    }
}

void ReactorTcpClient::handleError()
{
    QINMO_WARN("handleError has been called.");
    if (ClientState::kConnecting != state_)
    {
        QINMO_FATAL("Nonconnecting state in handleError. state=", (ClientState::kConnected == state_ ? "connected" : "disconnect"));
        std::exit(-1);
    }

    removeChannel();
    state_ = ClientState::kDisconnect;
    retry();
}
} // namespace qinmo::net
