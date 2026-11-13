#include "qinmo/net/http/Server.h"
#include "qinmo/tool/Logger.h"

namespace qinmo::net::http
{

Server::Server(EventLoop* loop, const InetAddr& listenAddr, unsigned int numThread, bool reusePort)
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
        [this](const Request& request, Response& response)
        {
            //;
        };
}

Server::~Server() { }



void Server::start()
{
    QINMO_INFO("Http Server is starting.");
    server_.start();
}

void Server::setHttpFunc(HttpFunc func)
{
    func_ = std::move(func);
}



void Server::onMessage(const RTcpConnPtr& conn, PacketBuffer& buffer, Timestamp timestamp)
{
    Context& context = contexts_.getContext(conn.get());

    if (!context.parse(buffer))
    {
        conn->send("HTTP/1.1 404 Bad Request\r\n\r\n");
        conn->shutdown();
    }

    if (context.isParsed())
    {
        conn->send("HTTP/1.1 200 OK\r\n\r\n");
        conn->shutdown();
        context.reset();
    }
}

void Server::connect(const RTcpConnPtr& conn)
{
    contexts_.add(conn.get());
}

void Server::disconnect(const RTcpConnPtr& conn)
{
    contexts_.release(conn.get());
}

} // namespace qinmo::net::http
