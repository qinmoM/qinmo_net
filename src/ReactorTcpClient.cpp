#include "qinmo/net/ReactorTcpClient.h"
#include "qinmo/net/EventLoop.h"
#include "qinmo/base/Logger.h"

namespace qinmo::net
{
namespace detail
{

ReactorTcpClientCore::ReactorTcpClientCore(EventLoop* loop, const InetAddr& serverAddr)
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
    QINMO_INFO("ReactorTcpClientCore create.");
}

ReactorTcpClientCore::~ReactorTcpClientCore()
{
    RTcpConnPtr conn;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        conn = connection_;
        connection_.reset();
    }

    if (conn)
        loop_->queueInLoop( [conn]() { conn->handleClose(); } );
    else
        stopConnecting();
}



void ReactorTcpClientCore::connect() 
{
    QINMO_INFO("Client begin connecting. serverIP: ", serverAddr_.getIP());

    needStart_.store(true);
    std::shared_ptr<ReactorTcpClientCore> self = shared_from_this();
    loop_->runInLoop( [self]() { self->start(); } );
}

void ReactorTcpClientCore::disconnect()
{
    needStart_.store(false);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (connection_)
            connection_->shutdown();
    }
}

void ReactorTcpClientCore::stopConnecting()
{
    needStart_.store(false);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (ClientState::kConnecting == state_)
        {
            state_ = ClientState::kDisconnect;
            removeChannel();
            sock_.close();
        }
    }
}



EventLoop* ReactorTcpClientCore::getEventLoop()
{
    return loop_;
}

bool ReactorTcpClientCore::isConnect() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return nullptr != connection_;
}

bool ReactorTcpClientCore::isRetry() const
{
    return isRetry_.load();
}

void ReactorTcpClientCore::setRetry(bool enable)
{
    isRetry_.store(enable);
}



void ReactorTcpClientCore::setConnectFunc(const ConnectFunc& f)
{
    connectFunc_ = f;
}

void ReactorTcpClientCore::setDisconnectFunc(const DisconnectFunc& f)
{
    disconnectFunc_ = f;
}

void ReactorTcpClientCore::setMessageFunc(const MessageFunc& f)
{
    messageFunc_ = f;
}

void ReactorTcpClientCore::setWriteCompleteFunc(const WriteCompleteFunc& f)
{
    writeCompleteFunc_ = f;
}



void ReactorTcpClientCore::removeChannel()
{
    channel_->disableAll();
    channel_->remove();
    std::shared_ptr<ReactorTcpClientCore> self = shared_from_this();
    loop_->queueInLoop( [self]() -> void { self->channel_.reset(); } );
}

void ReactorTcpClientCore::start()
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
            std::shared_ptr<ReactorTcpClientCore> self = shared_from_this();
            channel_->setWriteEvent([self]() { self->handleWrite(); });
            channel_->setErrorEvent([self]() { self->handleError(); });
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

void ReactorTcpClientCore::retry()
{
    state_ = ClientState::kDisconnect;
    sock_.close();
    if (isRetry_ && needStart_)
    {
        QINMO_INFO("Retry to connect ", serverAddr_.getIP(), ":", serverAddr_.getPort(), " in ", retryMs_, " milliseconds.");
        std::shared_ptr<ReactorTcpClientCore> self = shared_from_this();
        loop_->timerAfter(retryMs_ / 1000, [self]() -> void { self->start(); } );
        retryMs_ = std::min(retryMs_ * 2, kMaxRetryMs);
    }
    else
    {
        QINMO_DEBUG("Do not call retry.");
    }
}

void ReactorTcpClientCore::newConnect()
{
    TcpConnect sock(std::move(sock_));
    if (!sock.isValid())
    {
        QINMO_ERROR("Failed to attach - connect invalid. cfd=", sock.getfd());
        return;
    }

    InetAddr local = sock.getLocalAddr();
    InetAddr peer = sock.getPeerAddr();
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
    std::shared_ptr<ReactorTcpClientCore> self = shared_from_this();
    conn->setCloseFunc( [self]() -> void { self->removeConnect(); } );

    {
        std::lock_guard<std::mutex> lock(mutex_);
        connection_ = conn;
    }
    conn->connectEstablished();
}

void ReactorTcpClientCore::removeConnect()
{
    QINMO_INFO("Client connect being remove.");
    RTcpConnPtr conn;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        conn = connection_;
        connection_.reset();
    }

    loop_->queueInLoop( [conn]() -> void { conn->connectDestroyed(); } );
    if (isRetry() && needStart_.load())
    {
        QINMO_INFO("Reconnect to ", serverAddr_.getIP(), ':', serverAddr_.getPort(), '.');
        retryMs_ = kInitRetryMs;
        state_ = ClientState::kDisconnect;
        start();
    }
}



void ReactorTcpClientCore::handleWrite()
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

void ReactorTcpClientCore::handleError()
{
    QINMO_WARN("handleError has been called.");
    if (ClientState::kConnecting != state_)
    {
        QINMO_ERROR("???bug???Nonconnecting state in handleError. state=", (ClientState::kConnected == state_ ? "connected" : "disconnect"));
        return;
    }

    removeChannel();
    state_ = ClientState::kDisconnect;
    retry();
}
} // namespace detail


} // namespace qinmo::net
