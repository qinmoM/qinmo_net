#include "qinmo/net/SocketUDP.h"

namespace qinmo::net
{

SocketUDP SocketUDP::createRaw(const InetAddr& addr, SockFlags flags)
{
    if (!addr.isValid())
        return SocketUDP();

    return SocketUDP(detail::socket((addr.isIPv4() ? AF_INET : AF_INET6), SOCK_DGRAM | static_cast<int>(flags)), true);
}

SocketUDP SocketUDP::createNonBlockOrDie(const InetAddr& addr)
{
    return createRaw(addr, SockFlags::NonBlocking | SockFlags::CloseOnExec);
}

SocketUDP SocketUDP::attach(const SocketType fd)
{
    if (SOCK_DGRAM != detail::getSocketType(fd))
        return SocketUDP();

    return SocketUDP(fd, true);
}

SocketUDP SocketUDP::borrow(const SocketType fd)
{
    if (SOCK_DGRAM != detail::getSocketType(fd))
        return SocketUDP();

    return SocketUDP(fd, false);
}



SocketUDP::SocketUDP() { }

SocketUDP::SocketUDP(SocketType fd, bool owns)
    : sockfd_(fd)
    , owns_(owns)
{ }

SocketUDP::~SocketUDP()
{
    close();
}

SocketUDP::SocketUDP(SocketUDP&& other) noexcept
{
    close();

    sockfd_ = other.sockfd_;
    state_ = other.state_;
    owns_ = other.owns_;
    other.sockfd_ = g_SocketTypeEmpty;
    other.state_ = 0;
    other.owns_ = false;
}

SocketUDP& SocketUDP::operator=(SocketUDP&& other) noexcept
{
    close();

    sockfd_ = other.sockfd_;
    state_ = other.state_;
    owns_ = other.owns_;
    other.sockfd_ = g_SocketTypeEmpty;
    other.state_ = 0;
    other.owns_ = false;
    return *this;
}



bool SocketUDP::isValid() const
{
    return g_SocketTypeEmpty != sockfd_;
}

bool SocketUDP::isOwns() const
{
    return owns_;
}

SocketType SocketUDP::getfd() const
{
    return sockfd_;
}

bool SocketUDP::isBind() const
{
    return state_ & IsBind;
}

bool SocketUDP::isConnect() const
{
    return state_ & IsConnect;
}

InetAddr SocketUDP::getLocalAddr() const
{
    if (!isBind())
        return InetAddr();

    detail::sockaddr addr;
    detail::zeroMemory(&addr, sizeof(addr));
    if (!detail::getsockname(sockfd_, addr))
        detail::sockaddr_cast<detail::sockaddr, ::sockaddr>(&addr)->sa_family = AF_UNSPEC;

    return InetAddr(addr);
}

InetAddr SocketUDP::getPeerAddr() const
{
    if (!isConnect())
        return InetAddr();

    detail::sockaddr addr;
    detail::zeroMemory(&addr, sizeof(addr));
    if (!detail::getpeername(sockfd_, addr))
        detail::sockaddr_cast<detail::sockaddr, ::sockaddr>(&addr)->sa_family = AF_UNSPEC;

    return InetAddr(addr);
}



ssize_t SocketUDP::recvfrom(char* buf, size_t len, InetAddr& peer)
{
    detail::sockaddr addr = peer.getSockaddr();
    ssize_t l = detail::recvfrom(sockfd_, buf, len, addr);
    peer = InetAddr(addr);
    return l;
}

ssize_t SocketUDP::sendto(const char* buf, size_t len, const InetAddr& peer)
{
    return detail::sendto(sockfd_, buf, len, peer.getSockaddr());
}

bool SocketUDP::bind(const InetAddr& local)
{
    if (!isValid() || isBind())
        return false;

    return detail::bind(sockfd_, local.getSockaddr());
}

bool SocketUDP::connect(const InetAddr& peer)
{
    if (!isValid() || isConnect())
        return false;

    return detail::connect(sockfd_, peer.getSockaddr());
}

ssize_t SocketUDP::recv(char* buf, size_t len)
{
    if (!isConnect())
    {
        errno = ENOTCONN;
        return -1;
    }

    return detail::recv(sockfd_, buf, len);
}

ssize_t SocketUDP::send(const char* buf, size_t len)
{
    if (!isConnect())
    {
        errno = ENOTCONN;
        return -1;
    }

    return detail::send(sockfd_, buf, len);
}

bool SocketUDP::close()
{
    if (!isOwns() || !isValid() || -1 == qinmo::net::detail::close(sockfd_))
        return false;

    sockfd_ = g_SocketTypeEmpty;
    state_ = 0;
    owns_ = false;
    return true;
}



bool SocketUDP::setReusePort(bool enable)
{
    return detail::setPortReuse(sockfd_, enable);
}

bool SocketUDP::setReuseAddr(bool enable)
{
    return detail::setAddrReuse(sockfd_, enable);
}

}