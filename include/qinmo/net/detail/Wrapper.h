/**
 * @brief sockaddr-related functions wrapper class
 * 
 * This is an internal header file, you should not include this.
 */

#pragma once




#if defined(__linux__)

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <endian.h>
#include <netinet/tcp.h>

#elif defined(_WIN32)

#include <winsock2.h>
#include <ws2tcpip.h>

#endif

#include <stdint.h>
#include "qinmo/base/detail/Common.h"
#include "../../base/StringView.h"



/// @namespace qinmo
namespace qinmo
{
/// @namespace qinmp::net
namespace net
{

#if defined(__linux__)

using ssize_t = ::ssize_t;
using size_t = ::size_t;

using SocketType = int;
constexpr SocketType g_SocketTypeEmpty = -1;

enum class SockFlags : int
{
    None = 0,
    NonBlocking = SOCK_NONBLOCK,
    CloseOnExec = SOCK_CLOEXEC
};

#elif defined(_WIN32)

using ssize_t = ::ptrdiff_t;
using size_t = ::size_t;

using SocketType = SOCKET;
constexpr SocketType g_SocketTypeEmpty = INVALID_SOCKET;

enum class SockFlags : int
{
    None = 0,
    NonBlocking = 1,
    CloseOnExec = None
};

#endif

constexpr SockFlags operator|(const SockFlags& a, const SockFlags& b)
{
    return static_cast<SockFlags>(static_cast<const int>(a) | static_cast<const int>(b));
}

constexpr SockFlags operator|=(SockFlags& a, const SockFlags& b)
{
    a = a | b;
    return a;
}

/// @namespace qinmo::net::detail
/// @warning For internal use only, do NOT use it from outside the library
namespace detail
{

using sockaddr_in = struct ::sockaddr_in;
using sockaddr_in6 = struct ::sockaddr_in6;
union sockaddr
{
    sockaddr_in addr4_;
    sockaddr_in6 addr6_;
};

/// @brief convert sockaddr pointer to/from sockaddr_in/sockaddr_in6 pointer
/// @tparam from type of t parameter
/// @tparam to target type
/// @note sockaddr_in and sockaddr_in6 are mutually non-convertible
template <typename from, typename to>
to* sockaddr_cast(from* t)
{
    static_assert(false, "sockaddr_cast : unspecialized types.");
    return nullptr;
}
/// @details template specialization
template <>
inline const sockaddr* sockaddr_cast(const sockaddr_in* addr) { return static_cast<const sockaddr*>(static_cast<const void*>(addr)); }
template <>
inline const sockaddr* sockaddr_cast(const sockaddr_in6* addr) { return static_cast<const sockaddr*>(static_cast<const void*>(addr)); }
template <>
inline sockaddr* sockaddr_cast(sockaddr_in* addr) { return static_cast<sockaddr*>(static_cast<void*>(addr)); }
template <>
inline sockaddr* sockaddr_cast(sockaddr_in6* addr) { return static_cast<sockaddr*>(static_cast<void*>(addr)); }
template <>
inline const sockaddr_in* sockaddr_cast(const sockaddr* addr) { return &addr->addr4_; }
template <>
inline const sockaddr_in6* sockaddr_cast(const sockaddr* addr) { return &addr->addr6_; }
template <>
inline sockaddr_in* sockaddr_cast(sockaddr* addr) { return &addr->addr4_; }
template <>
inline sockaddr_in6* sockaddr_cast(sockaddr* addr) { return &addr->addr6_; }
/// @brief detail::sockaddr convert to generic sockaddr
template <>
inline ::sockaddr* sockaddr_cast(sockaddr* addr) { return static_cast<::sockaddr*>(static_cast<void*>(addr)); }
template <>
inline const ::sockaddr* sockaddr_cast(const sockaddr* addr) { return static_cast<const ::sockaddr*>(static_cast<const void*>(addr)); }
template <>
inline sockaddr* sockaddr_cast(::sockaddr* addr) { return static_cast<sockaddr*>(static_cast<void*>(addr)); }
template <>
inline const sockaddr* sockaddr_cast(const ::sockaddr* addr) { return static_cast<const sockaddr*>(static_cast<const void*>(addr)); }

#if defined(__linux__)
/// @brief network to host byte order, supporting 16-bit, 32-bit and 64-bit unsigned intergers
/// @param net network order
inline uint16_t netToHost16(uint16_t net) { return be16toh(net); }
inline uint32_t netToHost32(uint32_t net) { return be32toh(net); }
inline uint64_t netToHost64(uint64_t net) { return be64toh(net); }

/// @brief host to network byte order, supporting 16-bit, 32-bit and 64-bit unsigned intergers
/// @param host host order
inline uint16_t hostToNet16(uint16_t host) { return htobe16(host); }
inline uint32_t hostToNet32(uint32_t host) { return htobe32(host); }
inline uint64_t hostToNet64(uint64_t host) { return htobe64(host); }
#elif defined(_WIN32)
/// @brief network to host byte order, supporting 16-bit, 32-bit and 64-bit unsigned intergers
/// @param net network order
inline uint16_t netToHost16(uint16_t net) { return ntohs(net); }
inline uint32_t netToHost32(uint32_t net) { return ntohl(net); }
inline uint64_t netToHost64(uint64_t net)
{
    constexpr uint16_t test = 0x0001;
    const uint8_t temp = *(static_cast<const uint8_t*>(static_cast<const void*>(&test)));

    if (!temp)
        return net;
    
    uint32_t high = ntohl(static_cast<uint32_t>(net >> 32));
    uint32_t low = ntohl(static_cast<uint32_t>(net & 0xffffffff));
    return static_cast<uint64_t>(low) << 32 | high;
}

/// @brief host to network byte order, supporting 16-bit, 32-bit and 64-bit unsigned intergers
/// @param host host order
inline uint16_t hostToNet16(uint16_t host) { return htons(host); }
inline uint32_t hostToNet32(uint32_t host) { return htonl(host); }
inline uint64_t hostToNet64(uint64_t host) { return netToHost64(host); }
#endif


/// @brief initialize this memory block to zero
/// @param buf pointer
/// @param len length
inline void zeroMemory(void* buf, size_t len) { ::memset(buf, 0, len); }

/// @brief convert dotted decimal to network byte order
/// @param cp dotted decimal string
/// @param addr net address
/// @return return true when successful, otherwise false
inline bool pton4(StringView cp, in_addr& addr) { return 1 == ::inet_pton(AF_INET, cp.data(), &addr); }
inline bool pton6(StringView cp, in6_addr& addr) { return 1 == ::inet_pton(AF_INET6, cp.data(), &addr); }
/// @brief convert network byte order to dotted decimal
/// @param addr net address
/// @return return dotted decimal string
inline std::string ntop4(const in_addr& addr) { char buf[INET_ADDRSTRLEN]; return ::inet_ntop(AF_INET, &addr, buf, sizeof(buf)); }
inline std::string ntop6(const in6_addr& addr) { char buf[INET6_ADDRSTRLEN]; return ::inet_ntop(AF_INET6, &addr, buf, sizeof(buf)); }

#if defined(__linux__)

inline bool getsockname(SocketType sockfd, sockaddr& addr) { socklen_t len = sizeof(addr); return 0 == ::getsockname(sockfd, sockaddr_cast<sockaddr, ::sockaddr>(&addr), &len); }
inline bool getpeername(SocketType sockfd, sockaddr& addr) { socklen_t len = sizeof(addr); return 0 == ::getpeername(sockfd, sockaddr_cast<sockaddr, ::sockaddr>(&addr), &len); }

inline SocketType socket(int af, int type, int protocol = 0) { return ::socket(af, type, protocol); }
inline bool bind(SocketType sockfd, const sockaddr& addr) { return 0 == ::bind(sockfd, sockaddr_cast<const sockaddr, const ::sockaddr>(&addr), sizeof(addr)); }
inline bool listen(SocketType sockfd, int num = 128){ return 0 == ::listen(sockfd, num); };
inline SocketType accept(SocketType sockfd, sockaddr& addr, int flags = 0) { socklen_t len = sizeof(addr); return ::accept4(sockfd, sockaddr_cast<sockaddr, ::sockaddr>(&addr), &len, flags); }
inline bool connect(SocketType sockfd, const sockaddr& addr) { return 0 == ::connect(sockfd, sockaddr_cast<const sockaddr, const ::sockaddr>(&addr), sizeof(addr)); }
inline ssize_t send(SocketType sockfd, const void* buf, size_t count) { return ::send(sockfd, buf, count, 0); }
inline ssize_t recv(SocketType sockfd, void* buf, size_t count) { return ::recv(sockfd, buf, count, 0); }
inline ssize_t sendto(SocketType sockfd, const void* buf, size_t count, const sockaddr& addr) { return ::sendto(sockfd, buf, count, 0, sockaddr_cast<const sockaddr, const ::sockaddr>(&addr), sizeof(sockaddr)); }
inline ssize_t recvfrom(SocketType sockfd, void* buf, size_t count, sockaddr& addr) { unsigned int len = sizeof(addr); return ::recvfrom(sockfd, buf, count, 0, sockaddr_cast<sockaddr, ::sockaddr>(&addr), &len); }
inline bool shutdownWrite(SocketType sockfd) { return 0 == ::shutdown(sockfd, SHUT_WR); }
inline int close(SocketType sockfd) { return qinmo::detail::close(fd); }

inline bool isAddrReuse(SocketType sockfd) { int opt = 0; socklen_t len = sizeof(opt); return 0 == ::getsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, &len) && 1 == opt; }

inline int getSockOpt(SocketType sockfd, int level, int optname, void* opt, socklen_t& len) { return ::getsockopt(sockfd, level, optname, opt, &len); }
inline bool setAddrReuse(SocketType sockfd, bool enable) { int opt = enable ? 1 : 0; return 0 == ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); }
inline bool setPortReuse(SocketType sockfd, bool enable) { int opt = enable ? 1 : 0; return 0 == ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)); }
inline bool setTcpNoDelay(SocketType sockfd, bool enable) { int opt = enable ? 1 : 0; return 0 == ::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)); }
inline bool setKeepAlive(SocketType sockfd, bool enable) { int opt = enable ? 1 : 0; return 0 == ::setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt)); }
/// @return 0 if fails
inline int getSocketType(SocketType sockfd) { int opt = 0; socklen_t len = sizeof(opt); return (0 == ::getsockopt(sockfd, SOL_SOCKET, SO_TYPE, &opt, &len) ? opt : 0); }

#elif defined(_WIN32)

inline bool getsockname(SocketType sockfd, sockaddr& addr) { socklen_t len = sizeof(addr); return 0 == ::getsockname(sockfd, sockaddr_cast<sockaddr, ::sockaddr>(&addr), static_cast<int*>(&len)); }
inline bool getpeername(SocketType sockfd, sockaddr& addr) { socklen_t len = sizeof(addr); return 0 == ::getpeername(sockfd, sockaddr_cast<sockaddr, ::sockaddr>(&addr), static_cast<int*>(&len)); }

inline SocketType socket(int af, int type, int protocol = 0)
{
    bool isNonBlocking = type & static_cast<int>(SockFlags::NonBlocking);
    type &= ~(static_cast<int>(SockFlags::NonBlocking | SockFlags::CloseOnExec));
    SocketType sockfd = ::socket(af, type, protocol);

    if (g_SocketTypeEmpty == sockfd)
        return g_SocketTypeEmpty;

    if (isNonBlocking)
    {
        u_long mode = 1;
        ::ioctlsocket(sockfd, FIONBIO, &mode);
    }

    return sockfd;
}
inline bool bind(SocketType sockfd, const sockaddr& addr) { return 0 == ::bind(sockfd, sockaddr_cast<const sockaddr, const ::sockaddr>(&addr), sizeof(addr)); }
inline bool listen(SocketType sockfd, int num = 128){ return 0 == ::listen(sockfd, num); };
inline SocketType accept(SocketType sockfd, sockaddr& addr, int flags = 0)
{
    socklen_t len = sizeof(addr);
    SocketType fd = ::accept(sockfd, sockaddr_cast<sockaddr, ::sockaddr>(&addr), &len);

    if (g_SocketTypeEmpty == fd)
        return g_SocketTypeEmpty;

    if (flags & static_cast<int>(SockFlags::NonBlocking))
    {
        u_long mode = 1;
        ::ioctlsocket(fd, FIONBIO, &mode);
    }

    return fd;
}
inline bool connect(SocketType sockfd, const sockaddr& addr) { return 0 == ::connect(sockfd, sockaddr_cast<const sockaddr, const ::sockaddr>(&addr), sizeof(addr)); }
inline int send(SocketType sockfd, const char* buf, size_t count) { return ::send(sockfd, buf, count, 0); }
inline int recv(SocketType sockfd, char* buf, size_t count) { return ::recv(sockfd, buf, count, 0); }
inline int sendto(SocketType sockfd, const char* buf, size_t count, const sockaddr& addr) { return ::sendto(sockfd, buf, count, 0, sockaddr_cast<const sockaddr, const ::sockaddr>(&addr), sizeof(sockaddr)); }
inline int recvfrom(SocketType sockfd, char* buf, size_t count, sockaddr& addr) { int len = sizeof(addr); return ::recvfrom(sockfd, buf, count, 0, sockaddr_cast<sockaddr, ::sockaddr>(&addr), &len); }
inline bool shutdownWrite(SocketType sockfd) { return 0 == ::shutdown(sockfd, SD_SEND); }
inline int close(SocketType sockfd) { return ::closesocket(sockfd); }

inline bool isAddrReuse(SocketType sockfd) { int opt = 0; socklen_t len = sizeof(opt); return 0 == ::getsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, static_cast<char*>(static_cast<void*>(&opt)), &len) && 1 == opt; }

inline int getSockOpt(SocketType sockfd, int level, int optname, void* opt, socklen_t& len) { return ::getsockopt(sockfd, level, optname, static_cast<char*>(opt), &len); }
inline bool setAddrReuse(SocketType sockfd, bool enable) { int opt = enable ? 1 : 0; return 0 == ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, static_cast<char*>(static_cast<void*>(&opt)), sizeof(opt)); }
inline bool setPortReuse(SocketType sockfd, bool enable) { return false; }
inline bool setExclusiveAddrUse(SocketType sockfd, bool enable) { int opt = enable ? 1 : 0; return 0 == ::setsockopt(sockfd, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, static_cast<char*>(static_cast<void*>(&opt)), sizeof(opt)); }
inline bool setTcpNoDelay(SocketType sockfd, bool enable) { int opt = enable ? 1 : 0; return 0 == ::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, static_cast<char*>(static_cast<void*>(&opt)), sizeof(opt)); }
inline bool setKeepAlive(SocketType sockfd, bool enable) { int opt = enable ? 1 : 0; return 0 == ::setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, static_cast<char*>(static_cast<void*>(&opt)), sizeof(opt)); }
/// @return 0 if fails
inline int getSocketType(SocketType sockfd) { int opt = 0; socklen_t len = sizeof(opt); return (0 == ::getsockopt(sockfd, SOL_SOCKET, SO_TYPE, static_cast<char*>(static_cast<void*>(&opt)), &len) ? opt : 0); }

#endif

} // namespace detail
} // namespace net
} // namespace qinmo