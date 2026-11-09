#pragma once

#include "qinmo/net/ReactorTcpServer.h"
#include "qinmo/net/http/HttpContextsManager.h"

namespace qinmo
{

namespace net
{

class HttpRequest;
class HttpResponse;

/// @brief http request handle callback, make a corresponding response
using HttpFunc = std::function<void(const HttpRequest&, HttpResponse&)>;

class HttpServer
{
public:
    HttpServer(EventLoop* loop, const InetAddr& listenAddr, unsigned int numThread = 0, bool reusePort = false);
    ~HttpServer();

    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;

    HttpServer(HttpServer&&) = delete;
    HttpServer& operator=(HttpServer&&) = delete;

public:
    void start();

private:
    ReactorTcpServer server_;
    HttpFunc func_;
    HttpContextsManager contexts_;

};
} // namespace net
} // namespace qinmo
