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
    bool isRetry();
    void setRetry(bool enable);

    void setConnectFunc(const ConnectFunc& f);
    void setDisconnectFunc(const DisconnectFunc& f);
    void setMessageFunc(const MessageFunc& f);
    void setWriteCompleteFunc(const WriteCompleteFunc& f);

private:
    EventLoop* loop_;
    SocketTCP sock_;
    InetAddr serverAddr_;
    bool isRetry_;
    bool isConnecting_;

    ConnectFunc connect_;
    DisconnectFunc disconnect_;
    MessageFunc message_;
    WriteCompleteFunc writeComplete_;

    RTcpConnPtr connection_;

};
} // namespace qinmo::net