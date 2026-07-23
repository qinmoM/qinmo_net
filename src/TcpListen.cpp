#include "qinmo/net/TcpListen.h"

/// @namespace qinmo
namespace qinmo
{
/// @namespace qinmp::net
namespace net
{

TcpListen TcpListen::createRaw(const InetAddr& addr, SockFlags flags)
{
    return TcpListen(SocketTCP::createRaw(addr, flags));
}

TcpListen TcpListen::createNonBlockOrDie(const InetAddr& addr)
{
    return TcpListen::createRaw(addr, SockFlags::NonBlocking | SockFlags::CloseOnExec);
}



TcpListen::TcpListen()
    : sock_()
    , state_(TcpListenState::Created)
{ }

TcpListen::TcpListen(SocketTCP&& sock)
    : sock_(std::move(sock))
    , state_(TcpListenState::Created)
{ }



bool TcpListen::isValid() const
{
    return sock_.isValid();
}

bool TcpListen::isBind() const
{
    return state_ >= TcpListenState::Bound;
}

bool TcpListen::isListen() const
{
    return state_ >= TcpListenState::Listening;
}

int TcpListen::getfd() const
{
    return sock_.getfd();
}

InetAddr TcpListen::getLocalAddr() const
{
    return sock_.getLocalAddr();
}

InetAddr TcpListen::getPeerAddr() const
{
    return sock_.getPeerAddr();
}



ssize_t TcpListen::recv(char* buf, size_t len)
{
    return sock_.recv(buf, len);
}

ssize_t TcpListen::send(const char* buf, size_t len)
{
    return sock_.send(buf, len);
}

bool TcpListen::bind(const InetAddr& addr)
{
    if (TcpListenState::Created != state_ || !sock_.bind(addr))
        return false;

    state_ = TcpListenState::Bound;
    return true;
}

bool TcpListen::listen(int num)
{
    if (TcpListenState::Bound != state_ || !sock_.listen(num))
        return false;

    state_ = TcpListenState::Listening;
    return true;
}

TcpConnect TcpListen::accept(InetAddr& addr, SockFlags flags)
{
    if (TcpListenState::Listening != state_)
        return TcpConnect();

    return TcpConnect(sock_.accept(addr, flags));
}

TcpConnect TcpListen::acceptNonBlockOrDie(InetAddr &addr)
{
    return accept(addr, SockFlags::NonBlocking | SockFlags::CloseOnExec);
}

bool TcpListen::close()
{
    return sock_.close();
}



bool TcpListen::setReusePort(bool enable)
{
    if (TcpListenState::Created > state_ || TcpListenState::Bound <= state_)
        return false;

    return sock_.setReusePort(enable);
}

bool TcpListen::setReuseAddr(bool enable)
{
    if (TcpListenState::Created > state_ || TcpListenState::Bound <= state_)
        return false;

    return sock_.setReuseAddr(enable);
}


} // namespace net
} // namespace qinmo