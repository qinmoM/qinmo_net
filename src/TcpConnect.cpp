#include "qinmo/net/TcpConnect.h"

/// @namespace qinmo
namespace qinmo
{
/// @namespace qinmp::net
namespace net
{

TcpConnect TcpConnect::connectRaw(const InetAddr &serverAddr, SockFlags flags)
{
    SocketTCP sock = SocketTCP::createRaw(serverAddr, flags);
    if (!sock.isValid() || !sock.connect(serverAddr))
        return TcpConnect();

    return TcpConnect(std::move(sock));
}

TcpConnect TcpConnect::connectNonBlockOrDie(const InetAddr &serverAddr)
{
    return TcpConnect::connectRaw(serverAddr, SockFlags::NonBlocking | SockFlags::CloseOnExec);
}

TcpConnect::TcpConnect() : sock_() { }

TcpConnect::TcpConnect(SocketTCP&& sock) : sock_(std::move(sock)) { }

TcpConnect::~TcpConnect() { }


bool TcpConnect::isValid() const
{
    return sock_.isValid();
}

int TcpConnect::getfd() const
{
    return sock_.getfd();
}

InetAddr TcpConnect::getLocalAddr() const
{
    return sock_.getLocalAddr();
}

InetAddr TcpConnect::getPeerAddr() const
{
    return sock_.getPeerAddr();
}


ssize_t TcpConnect::recv(char *buf, size_t len)
{
    return sock_.recv(buf, len);
}

ssize_t TcpConnect::send(const char *buf, size_t len)
{
    return sock_.send(buf, len);
}

bool TcpConnect::shutdownWrite()
{
    return sock_.shutdownWrite();
}

bool TcpConnect::close()
{
    return sock_.close();
}


bool TcpConnect::setTcpNoDelay(bool enable)
{
    return sock_.setTcpNoDelay(enable);
}

bool TcpConnect::setKeepAlive(bool enable)
{
    return sock_.setKeepAlive(enable);
}

} // namespace net
} // namespace qinmo