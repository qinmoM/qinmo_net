#include "qinmo/net/http/HttpServer.h"
#include "qinmo/tool/Logger.h"

namespace qinmo
{
namespace net
{

HttpServer::HttpServer(EventLoop* loop, const InetAddr& listenAddr, unsigned int numThread = 0, bool reusePort = false)
    : server_(loop, listenAddr, numThread, reusePort)
{
    server_.setConnectFunc(
        [this](const RTcpConnPtr& conn)
        {
            connect(conn);
        });
    server_.setDisconnectFunc(
        [this](const RTcpConnPtr& conn)
        {
            disconnect(conn);
        });
    server_.setMessageFunc(
        [this](const RTcpConnPtr& conn, PacketBuffer& buffer, Timestamp timestamp)
        {
            onMessage(conn, buffer, timestamp);
        });
    func_ = 
        [this](const HttpRequest& request, HttpResponse& response)
        {
            //;

        };
}

HttpServer::~HttpServer()
{
    ;
}



void HttpServer::start()
{
    QINMO_INFO("Http Server is starting.");
    server_.start();
}

void HttpServer::setHttpFunc(HttpFunc func)
{
    func_ = std::move(func);
}



void HttpServer::onMessage(const RTcpConnPtr& conn, PacketBuffer& buffer, Timestamp timestamp)
{
    HttpContext& context = contexts_.getContext(conn.get());

    if (!context.parse(buffer))
    {
        //
        conn->shutdown();
    }

    if (context.isParsed())
    {
        // 
        context.reset();
    }
}

void HttpServer::connect(const RTcpConnPtr& conn)
{
    contexts_.add(conn.get());
}

void HttpServer::disconnect(const RTcpConnPtr& conn)
{
    contexts_.release(conn.get());
}

} // namespace net
} // namespace qinmo
