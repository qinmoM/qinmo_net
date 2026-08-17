#pragma once

#include "SocketTCP.h"

/// @namespace qinmo
namespace qinmo
{
/// @namespace qinmp::net
namespace net
{

class TcpListen;
#if defined(__linux__)
class detail::ReactorTcpClientCore;
#endif

/// @brief encapsulate client socket and connect socket
/// @note you must specify whether to Create a client use server address or Receive from listen socket
/// @note   - TcpConnect conn = TcpConnect::connectRaw(InetAddr())
/// @note   - TcpConnect conn = TcpListen.accept()
class TcpConnect
{
public:
    /// @brief create a client socket from remote server
    /// @param serverAddr remote server address
    /// @return using the move constructor
    /// @note better to check whether the returned value is valid : Call isValid()
    static TcpConnect connectRaw(const InetAddr& serverAddr, SockFlags flags = SockFlags::None);
    /// @brief equal to connectRaw(InetAddr(), SockFlags::NonBlocking | SockFlags::CloseOnExec)
    /// @note better to check whether the returned value is valid: call isValid()
    static TcpConnect connectNonBlockOrDie(const InetAddr& serverAddr);

public:
    TcpConnect();
    ~TcpConnect();

    TcpConnect(const TcpConnect&) = delete;
    TcpConnect& operator=(const TcpConnect&) = delete;

    TcpConnect(TcpConnect&&) noexcept = default;
    TcpConnect& operator=(TcpConnect&&) noexcept = default;

public:
    /// @return return true it has been initialized
    bool isValid() const;
    /// @brief get current file descriptor
    int getfd() const;
    /// @brief get local address
    /// @note need to Check if the returned InetAddr is valid : Call functions isIPv4() and isIPv6()
    InetAddr getLocalAddr() const;
    /// @brief get peer address
    /// @note need to Check if the returned InetAddr is valid : Call functions isIPv4() and isIPv6()
    InetAddr getPeerAddr() const;

    ssize_t recv(char* buf, size_t len);
    ssize_t send(const char* buf, size_t len);
    /// @brief half-close (close write and send FIN to peer)
    bool shutdownWrite();
    /// @note isValid function return false After call close
    bool close();

    /// @brief set whether Nagle algorithm (TCP packet buffering)
    bool setTcpNoDelay(bool enable);
    /// @brief set auto send heartbeat packets
    /// @note recommend to implement heartbeat packets at the application layer
    bool setKeepAlive(bool enable);

private:
    friend class TcpListen;
#if defined(__linux__)
    friend class detail::ReactorTcpClientCore;
#endif

    TcpConnect(SocketTCP&& sock);

private:
    SocketTCP sock_;

};
} // namespace net
} // namespace qinmo