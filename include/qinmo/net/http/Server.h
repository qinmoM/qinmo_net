#pragma once

#include "qinmo/net/http/ContextsManager.h"

namespace qinmo::net::http
{


class Context;

using RequestHeadersFunc = ContextFunc;
using RequestMessageFunc = MessageFunc;
using RequestCompleteFunc = ContextFunc;
using ResponseBodyFunc = ContextFunc;



class Server
{
public:
    Server(EventLoop* loop, const InetAddr& listenAddr, unsigned int numThread = 0, bool reusePort = false);
    ~Server();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    Server(Server&&) = delete;
    Server& operator=(Server&&) = delete;

public:
    void start();

    /// @note Triggered when client's headers has been receivedddd
    void setHeadersFunc(RequestHeadersFunc func);
    /// @note Triggered when client sends body message
    void setMessageFunc(RequestMessageFunc func);
    /// @note Triggered when client completes sending the message
    void setCompleteFunc(RequestCompleteFunc func);
    /// @note Triggered when client headers has been received and buffer is ready
    void setResponseFunc(ResponseBodyFunc func);

private:
    void onMessage(const RTcpConnPtr& conn, PacketBuffer& buffer, Timestamp timestamp);
    void connect(const RTcpConnPtr& conn);
    void disconnect(const RTcpConnPtr& conn);

private:
    ReactorTcpServer server_;
    ContextsManager contexts_;

    RequestHeadersFunc headersFunc_;
    RequestMessageFunc messageFunc_;
    RequestCompleteFunc completeFunc_;
    ResponseBodyFunc bodyFunc_;

    std::size_t waterMark_ = 1024 * 1024 * 1;

};
} // namespace qinmo::net::http
