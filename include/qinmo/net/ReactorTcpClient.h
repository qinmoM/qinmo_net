#pragma once

#include "ReactorTcpConnection.h"

namespace qinmo::net
{

namespace detail
{
class ReactorTcpClientCore;
} // namespace detail


class ReactorTcpClient
{
public:
    ReactorTcpClient(EventLoop* loop, const InetAddr& serverAddr);

    ReactorTcpClient(const ReactorTcpClient&) = delete;
    ReactorTcpClient& operator=(const ReactorTcpClient&) = delete;

    ReactorTcpClient(ReactorTcpClient&&) = delete;
    ReactorTcpClient& operator=(ReactorTcpClient&&) = delete;

public:
    void connect();
    void disconnect();
    /// @note has no effect after connection succeeds
    void stopConnecting();

    EventLoop* getEventLoop();
    bool isConnect() const;
    bool isRetry() const;
    void setRetry(bool enable);

    void setConnectFunc(const ConnectFunc& f);
    void setDisconnectFunc(const DisconnectFunc& f);
    void setMessageFunc(const MessageFunc& f);
    void setWriteCompleteFunc(const WriteCompleteFunc& f);

private:
    std::shared_ptr<detail::ReactorTcpClientCore> core_;

};



namespace detail
{

class ReactorTcpClientCore
    // Inherit this template class  when  needing to create shared_ptr via 'this' pointer inside the class
    : public std::enable_shared_from_this<ReactorTcpClientCore>
{
public:
    ReactorTcpClientCore(EventLoop* loop, const InetAddr& serverAddr);
    ~ReactorTcpClientCore();

    ReactorTcpClientCore(const ReactorTcpClientCore&) = delete;
    ReactorTcpClientCore& operator=(const ReactorTcpClientCore&) = delete;

    ReactorTcpClientCore(ReactorTcpClientCore&&) = delete;
    ReactorTcpClientCore& operator=(ReactorTcpClientCore&&) = delete;

public:
    void connect();
    void disconnect();
    /// @note has no effect after connection succeeds
    void stopConnecting();

    EventLoop* getEventLoop();
    bool isConnect() const;
    bool isRetry() const;
    void setRetry(bool enable);

    void setConnectFunc(const ConnectFunc& f);
    void setDisconnectFunc(const DisconnectFunc& f);
    void setMessageFunc(const MessageFunc& f);
    void setWriteCompleteFunc(const WriteCompleteFunc& f);

private:
    void handleWrite();
    void handleError();

    void removeChannel();
    void start();
    void retry();
    void newConnect();
    void removeConnect();

private:
    enum class ClientState
    {
        kDisconnect,
        kConnecting,
        kConnected
    };
    static constexpr int kInitRetryMs = 500;        // ms
    static constexpr int kMaxRetryMs = 1000 * 30;   // ms

    EventLoop* loop_;
    SocketTCP sock_;
    InetAddr serverAddr_;
    ClientState state_;
    std::atomic<bool> isRetry_;
    std::atomic<bool> needStart_;   // used to determine whether restart condition are met or client need to connect.
    mutable std::mutex mutex_;
    int retryMs_;

    ConnectFunc connectFunc_;
    DisconnectFunc disconnectFunc_;
    MessageFunc messageFunc_;
    WriteCompleteFunc writeCompleteFunc_;

    std::unique_ptr<Channel> channel_;
    RTcpConnPtr connection_;

};
} // namespace detail
} // namespace qinmo::net