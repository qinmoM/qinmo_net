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
///
///     - SocketTCP sock1 = SocketTCP::createRaw(InetAddr(), SockFlags::NonBlocking | SockFlags::CloseOnExec)
///
///     - SocketTCP sock2(SocketTCP::attach(1))
class SocketTCP
{
public:
    /// @brief create a new socket, will be closed upon release
    /// @param addr only use protocol, you must re-bind() after created
    /// @note better to check whether the returned value is valid : Call function isValid()
    /// @note has no  SockFlags::CloseOnExec  in the Windows Platform
    static SocketTCP createRaw(const InetAddr& addr, SockFlags flags = SockFlags::None);
    /// @brief equal to createRaw(InetAddr(), SockFlags::NonBlocking | SockFlags::CloseOnExec)
    static SocketTCP createNonBlockOrDie(const InetAddr& addr);
    /// @brief attach an existing socket, will be closed upon release
    /// @param fd file descriptor
    /// @note must check whether the returned value is valid : Call function isValid()
    static SocketTCP attach(const SocketType fd);
    /// @brief create a view, won't be closed upon release, and close() cannot be call
    /// @param fd file descriptor
    /// @note must check whether the returned value is valid : Call function isValid()
    static SocketTCP borrow(const SocketType fd);

public:
    explicit SocketTCP();
    ~SocketTCP();

    SocketTCP(const SocketTCP&) = delete;
    SocketTCP& operator=(const SocketTCP&) = delete;

    SocketTCP(SocketTCP&& other) noexcept;
    SocketTCP& operator=(SocketTCP&& other) noexcept;

public:
    /// @return return true if has been initialized
    bool isValid() const;
    /// @return return true if will be closed upon release
    bool isOwns() const;
    /// @brief get current file descriptor
    SocketType getfd() const;
    /// @brief get local address
    /// @note need to Check if the returned InetAddr is valid : Call function isValid()
    InetAddr getLocalAddr() const;
    /// @brief get peer address
    /// @note need to Check if the returned InetAddr is valid : Call function isValid()
    InetAddr getPeerAddr() const;

    ssize_t recv(char* buf, size_t len);
    ssize_t send(const char* buf, size_t len);
    /// @brief bind local address
    bool bind(const InetAddr& addr);
    /// @brief listen client socket
    bool listen(int num = 128);
    /// @return a new SocketTCP object
    /// @note must check whether InetAddr and SocketTCP is invalid
    SocketTCP accept(InetAddr& addr, SockFlags flags = SockFlags::None);
    /// @brief equal to accept(addr, SockFlags::NonBlocking | SockFlags::CloseOnExec)
    SocketTCP acceptNonBlockOrDie(InetAddr& addr);
    /// @brief connect server
    /// @param addr server address
    bool connect(const InetAddr& addr);
    /// @brief half-close (close write and send FIN to peer)
    bool shutdownWrite();
    bool close();

    /// @brief set whether Nagle algorithm (TCP packet buffering)
    bool setTcpNoDelay(bool enable);
    /// @brief set whether to enable port reuse
    bool setReusePort(bool enable);
    /// @brief set whether to enable addres reuse
    bool setReuseAddr(bool enable);
    /// @brief set auto send heartbeat packets
    /// @note recommend to implement heartbeat packets at the application layer
    bool setKeepAlive(bool enable);

private:
    SocketTCP(SocketType fd, bool owns);

private:
    SocketType sockfd_ = g_SocketTypeEmpty;
    bool owns_ = false;

};

} // namespace net
} // namespace qinmo