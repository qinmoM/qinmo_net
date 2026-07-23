#pragma once

#include "TcpConnect.h"

/// @namespace qinmo
namespace qinmo
{
/// @namespace qinmp::net
namespace net
{

/// @brief encapsulate listen socket
/// @note example: TcpListen listen = TcpListen::createRaw(InetAddr(), SockFlags::NonBlocking | SockFlags::CloseOnExec);
class TcpListen
{
public:
    /// @brief create a listen socket
    /// @return use move construct
    /// @note better to check whether the returned value is valid: call isValid()
    static TcpListen createRaw(const InetAddr& addr, SockFlags flags = SockFlags::None);
    /// @brief equal to createRaw(addr, SockFlags::NonBlocking | SockFlags::CloseOnExec)
    /// @note better to check whether the returned value is valid: call isValid()
    static TcpListen createNonBlockOrDie(const InetAddr& addr);

public:
    TcpListen();
    ~TcpListen() = default;

    TcpListen(const TcpListen&) = delete;
    TcpListen& operator=(const TcpListen&) = delete;

    TcpListen(TcpListen&&) noexcept = default;
    TcpListen& operator=(TcpListen&&) noexcept = default;

public:
    /// @return return true it has been initialized
    bool isValid() const;
    bool isBind() const;
    bool isListen() const;
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
    /// @brief bind local address
    bool bind(const InetAddr& addr);
    /// @brief listen client socket
    bool listen(int num = 128);
    /// @return a new SocketTCP object
    /// @note must check whether InetAddr and SocketTCP is invalid
    TcpConnect accept(InetAddr& addr, SockFlags flags = SockFlags::None);
    /// @brief equal to accept(addr, SockFlags::NonBlocking | SockFlags::CloseOnExec)
    TcpConnect acceptNonBlockOrDie(InetAddr& addr);
    /// @note isValid function return false After call close
    bool close();

    /// @brief set whether to enable port reuse
    bool setReusePort(bool enable);
    /// @brief set whether to enable addres reuse
    bool setReuseAddr(bool enable);

private:
    TcpListen(SocketTCP&& sock);

    enum class TcpListenState : char
    {
        Created = 0,
        Bound = 1,
        Listening = 2
    };

private:
    SocketTCP sock_;
    TcpListenState state_;

};
} // namespace net
} // namespace qinmo