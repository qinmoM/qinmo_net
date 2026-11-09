#pragma once

#include "TcpListen.h"
#include "ReactorTcpConnection.h"

/// @namespace qinmo
namespace qinmo
{
/// @namespace qinmp::net
namespace net
{

class ReactorTcpServer
{
public:
    ReactorTcpServer(EventLoop* loop, const InetAddr& listenAddr, unsigned int numThread = 0, bool reusePort = false);
    ~ReactorTcpServer();

    ReactorTcpServer(const ReactorTcpServer&) = delete;
    ReactorTcpServer& operator=(const ReactorTcpServer&) = delete;

    ReactorTcpServer(ReactorTcpServer&&) = delete;
    ReactorTcpServer& operator=(ReactorTcpServer&&) = delete;

public:
    /// @brief start listen
    /// @param func the thread initilization function
    /// @note must start loop.loop()
    void start(EventLoopThread::EventLoopThreadInitFunc func = [](EventLoop *) -> void { });

    void setConnectFunc(const ConnectFunc& f);
    void setDisconnectFunc(const DisconnectFunc& f);
    void setMessageFunc(const MessageFunc& f);
    void setWriteCompleteFunc(const WriteCompleteFunc& f);

private:
    /// @brief create a new connect, and register to loop
    void newConnect(Timestamp time);
    /// @brief remove connect in loop and registry
    /// @note unregister in TcpServer immediately, but will not be deleted right now
    void removeConnect(const RTcpConnPtr& conn);

private:
    EventLoop* loop_;
    std::unordered_map<int, RTcpConnPtr> rConnects_;
    std::unique_ptr<EventLoopThreadPool> threadPool_;

    ConnectFunc connect_;
    DisconnectFunc disconnect_;
    MessageFunc message_;
    WriteCompleteFunc writeComplete_;

    TcpListen sock_;
    Channel acceptChannel_;
    std::atomic<bool> started_;
    std::atomic<int> numConn_;

};
} // namespace net
} // namespace qinmo