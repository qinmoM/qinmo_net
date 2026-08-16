#pragma once

#include "ReactorTcpConnection.h"

namespace qinmo::net
{

class ReactorTcpClient
{
public:
    ReactorTcpClient(EventLoop* loop, const InetAddr& serverAddr);
    ~ReactorTcpClient();

    ReactorTcpClient(const ReactorTcpClient&) = delete;
    ReactorTcpClient& operator=(const ReactorTcpClient&) = delete;

    ReactorTcpClient(ReactorTcpClient&&) = delete;
    ReactorTcpClient& operator=(ReactorTcpClient&&) = delete;

public:
    void connect();
    void disconnect();
    void stop();

    EventLoop* getEventLoop();
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
    std::mutex mutex_;
    int retryMs_;

    ConnectFunc connectFunc_;
    DisconnectFunc disconnectFunc_;
    MessageFunc messageFunc_;
    WriteCompleteFunc writeCompleteFunc_;

    std::unique_ptr<Channel> channel_;
    RTcpConnPtr connection_;

};
} // namespace qinmo::net