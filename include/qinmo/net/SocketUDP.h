#pragma once

#include "InetAddr.h"

/// @namespace qinmo
namespace qinmo
{
/// @namespace qinmp::net
namespace net
{

/// @brief encapsulate TCP socket
/// @note
///   default constructor is empty.
///   you must specify whether to Create a new instance or Attach to an existing one. Such as:
/// @note - SocketUDP sock1 = SocketUDP::createRaw(InetAddr(), SockFlags::NonBlocking | SockFlags::CloseOnExec)
/// @note - SocketUDP sock2(SocketUDP::attach(1))
class SocketUDP
{
public:
    /// @brief create a new socket
    /// @return using the move constructor
    /// @note better to check whether the returned value is valid : Call function isValid()
    /// @note the addr only use protocol, you must bind after created
    /// @note has no  SockFlags::CloseOnExec  in the windows
    static SocketUDP createRaw(const InetAddr& addr, SockFlags flags = SockFlags::None);
    /// @brief equal to createRaw(InetAddr(), SockFlags::NonBlocking | SockFlags::CloseOnExec)
    /// @note better to check whether the returned value is valid: call isValid()
    static SocketUDP createNonBlockOrDie(const InetAddr& addr);
    /// @brief attach an existing socket
    /// @param fd file descriptor
    /// @return using the move constructor
    /// @note must check whether the returned value is valid : Call function isValid()
    static SocketUDP attach(const SocketType fd);

public:
    SocketUDP();
    ~SocketUDP();

    SocketUDP(const SocketUDP&) = delete;
    SocketUDP& operator=(const SocketUDP&) = delete;

    SocketUDP(SocketUDP&& other) noexcept;
    SocketUDP& operator=(SocketUDP&& other) noexcept;

public:
    /// @return return true if has been initialized
    bool isValid() const;
    /// @brief get current file descriptor
    SocketType getfd() const;
    /// @brief return true if has been bind local address
    bool isBind() const;
    /// @brief return true if has been connect peer address
    bool isConnect() const;
    /// @brief get local address
    /// @note need to Check if the returned InetAddr is valid : Call functions isIPv4() and isIPv6()
    InetAddr getLocalAddr() const;
    /// @brief get peer address
    /// @note need to Check if the returned InetAddr is valid : Call functions isIPv4() and isIPv6()
    InetAddr getPeerAddr() const;

    ssize_t recvfrom(char* buf, size_t len, InetAddr& peer);
    ssize_t sendto(const char* buf, size_t len, const InetAddr& peer);
    /// @brief bind local address
    /// @param addr local address
    bool bind(const InetAddr& local);
    /// @brief bind peer address, can be called repeatedly
    /// @param addr peer address
    bool connect(const InetAddr& peer);
    /// @note only after connect
    ssize_t recv(char* buf, size_t len);
    /// @note only after connect
    ssize_t send(const char* buf, size_t len);
    bool close();

    /// @brief set whether to enable port reuse
    bool setReusePort(bool enable);
    /// @brief set whether to enable addres reuse
    bool setReuseAddr(bool enable);

private:
    SocketUDP(SocketType fd);

private:
    static constexpr uint8_t IsBind = 1;
    static constexpr uint8_t IsConnect = 2;

    SocketType sockfd_ = g_SocketTypeEmpty;
    uint8_t state_ = 0;

};

} // namespace net
} // namespace qinmo