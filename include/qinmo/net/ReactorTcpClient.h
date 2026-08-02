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

private:
    enum class ClientState
    {
        kDisconnect,
        kConnecting,
        kConnected
    };
    static constexpr int kInitRetryDelay = 500;         // ms
    static constexpr int kMaxRetryDelay = 1000 * 30;    // ms

    EventLoop* loop_;
    SocketTCP sock_;
    InetAddr serverAddr_;
    ClientState state_;
    std::atomic<bool> isRetry_;
    bool needRestart_;          // used to determine whether restart condition are met.

    ConnectFunc connect_;
    DisconnectFunc disconnect_;
    MessageFunc message_;
    WriteCompleteFunc writeComplete_;

    std::unique_ptr<Channel> channel_;
    RTcpConnPtr connection_;

};
} // namespace qinmo::net