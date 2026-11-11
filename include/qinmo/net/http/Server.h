#pragma once

#include "qinmo/net/ReactorTcpServer.h"
#include "qinmo/net/http/ContextsManager.h"

namespace qinmo::net::http
{

class Request;
class Response;

/// @brief http request handle callback, make a corresponding response
using HttpFunc = std::function<void(const Request&, Response&)>;


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

    void setHttpFunc(HttpFunc func);

private:
    void onMessage(const RTcpConnPtr& conn, PacketBuffer& buffer, Timestamp timestamp);
    void connect(const RTcpConnPtr& conn);
    void disconnect(const RTcpConnPtr& conn);

private:
    ReactorTcpServer server_;
    HttpFunc func_;
    ContextsManager contexts_;

};
} // namespace qinmo::net::http
