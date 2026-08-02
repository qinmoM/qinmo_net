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
    , needRestart_(true)
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

    needRestart_ = true;
    loop_->runInLoop(
        [this]()
        {
            if (!needRestart_)
            {
                QINMO_DEBUG("Restart condtion is not met.");
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
            //   retry(sockfd);
                    break;

                case EACCES:
                case EPERM:
                case EAFNOSUPPORT:
                case EALREADY:
                case EBADF:
                case EFAULT:
                case ENOTSOCK:
                    QINMO_ERROR("connect error.");
                    sock_.close();
                    break;

                default:
                    QINMO_ERROR("Unexpected error.");
                    sock_.close();
                    break;
            }
        }
    );
}

void ReactorTcpClient::disconnect()
{
    needRestart_ = false;
}

void ReactorTcpClient::stop()
{
    needRestart_ = false;
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
    connect_ = f;
}

void ReactorTcpClient::setDisconnectFunc(const DisconnectFunc& f)
{
    disconnect_ = f;
}

void ReactorTcpClient::setMessageFunc(const MessageFunc& f)
{
    message_ = f;
}

void ReactorTcpClient::setWriteCompleteFunc(const WriteCompleteFunc& f)
{
    writeComplete_ = f;
}



void ReactorTcpClient::handleWrite()
{
    ;
}

void ReactorTcpClient::handleError()
{
    ;
}
} // namespace qinmo::net
