#include "qinmo/net/SocketTCP.h"

namespace qinmo::net
{

SocketTCP SocketTCP::createRaw(const InetAddr& addr, SockFlags flags)
{
    if (!addr.isValid())
        return SocketTCP();

    return SocketTCP(detail::socket((addr.isIPv4() ? AF_INET : AF_INET6), SOCK_STREAM | static_cast<int>(flags)));
}

SocketTCP SocketTCP::createNonBlockOrDie(const InetAddr& addr)
{
    return createRaw(addr, SockFlags::NonBlocking | SockFlags::CloseOnExec);
}

SocketTCP SocketTCP::attach(const SocketType fd)
{
    if (SOCK_STREAM != detail::getSocketType(fd))
        return SocketTCP(g_SocketTypeEmpty);

    return SocketTCP(fd);
}

SocketTCP::SocketTCP() { }

SocketTCP::SocketTCP(SocketType fd)
    : sockfd_(fd)
{ }

SocketTCP::~SocketTCP()
{
    close();
}

SocketTCP::SocketTCP(SocketTCP&& other) noexcept
{
    sockfd_ = other.sockfd_;
    other.sockfd_ = g_SocketTypeEmpty;
}

SocketTCP& SocketTCP::operator=(SocketTCP&& other) noexcept
{
    sockfd_ = other.sockfd_;
    other.sockfd_ = g_SocketTypeEmpty;
    return *this;
}

bool SocketTCP::isValid() const
{
    return g_SocketTypeEmpty != sockfd_;
}

SocketType SocketTCP::getfd() const
{
    return sockfd_;
}

InetAddr SocketTCP::getLocalAddr() const
{
    detail::sockaddr addr;
    detail::zeroMemory(&addr, sizeof(addr));
    if (!detail::getsockname(sockfd_, addr))
        detail::sockaddr_cast<detail::sockaddr, sockaddr>(&addr)->sa_family = AF_UNSPEC;

    return InetAddr(addr);
}

InetAddr SocketTCP::getPeerAddr() const
{
    detail::sockaddr addr;
    detail::zeroMemory(&addr, sizeof(addr));
    if (!detail::getpeername(sockfd_, addr))
        detail::sockaddr_cast<detail::sockaddr, sockaddr>(&addr)->sa_family = AF_UNSPEC;

    return InetAddr(addr);
}

ssize_t  SocketTCP::recv(char* buf, size_t len)
{
    return detail::recv(sockfd_, buf, len);
}

ssize_t  SocketTCP::send(const char* buf, size_t len)
{
    return detail::send(sockfd_, buf, len);
}

bool SocketTCP::bind(const InetAddr& addr)
{
    if (!isValid() || !detail::bind(sockfd_, addr.getSockaddr()))
        return false;

    return true;
}

bool SocketTCP::listen(int num)
{
    if (!isValid() || !detail::listen(sockfd_, num))
        return false;

    return true;
}

SocketTCP SocketTCP::accept(InetAddr& addr, int flags)
{
    detail::sockaddr temp;
    detail::zeroMemory(&temp, sizeof(temp));
    int sockfd = detail::accept(sockfd_, temp, flags);
    addr = InetAddr(temp);
    return SocketTCP::attach(sockfd);
}

SocketTCP SocketTCP::acceptNonBlockOrDie(InetAddr &addr)
{
    return accept(addr, static_cast<int>(SockFlags::NonBlocking | SockFlags::CloseOnExec));
}

bool SocketTCP::connect(const InetAddr &addr)
{
    if (!isValid() || !detail::connect(sockfd_, addr.getSockaddr()))
        return false;

    return true;
}

bool SocketTCP::shutdownWrite()
{
    if (!isValid() || !detail::shutdownWrite(sockfd_))
        return false;

    return true;
}

bool SocketTCP::close()
{
    if (!isValid() || -1 == qinmo::net::detail::close(sockfd_))
        return false;

    sockfd_ = g_SocketTypeEmpty;
    return true;
}

bool SocketTCP::setTcpNoDelay(bool enable)
{
    return detail::setTcpNoDelay(sockfd_, enable);
}

bool SocketTCP::setReusePort(bool enable)
{
    return detail::setAddrReuse(sockfd_, enable);
}

bool SocketTCP::setReuseAddr(bool enable)
{
    return detail::setAddrReuse(sockfd_, enable);
}

bool SocketTCP::setKeepAlive(bool enable)
{
    return detail::setKeepAlive(sockfd_, enable);
}

}