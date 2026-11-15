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
}

Server::~Server() { }



void Server::start()
{
    QINMO_INFO("Http Server is starting.");
    server_.start();
}

void Server::setHeadersFunc(RequestHeadersFunc func)
{
    headersFunc_ = std::move(func);
}

void Server::setMessageFunc(RequestMessageFunc func)
{
    messageFunc_ = std::move(func);
}

void Server::setCompleteFunc(RequestCompleteFunc func)
{
    completeFunc_ = std::move(func);
}

void Server::setResponseFunc(ResponseBodyFunc func)
{
    bodyFunc_ = std::move(func);
}



void Server::onMessage(const RTcpConnPtr& conn, PacketBuffer& buffer, Timestamp timestamp)
{
    Context& context = contexts_.getContext(conn.get());

    Context::ServerHandle handle = context.parse(buffer, headersFunc_, messageFunc_, completeFunc_);

    if (Context::ServerHandle::Close == handle)
        conn->close();
    else if (Context::ServerHandle::Shutdown == handle)
        conn->shutdown();
    else if (Context::ServerHandle::Reset == handle)
        context.reset();
}

void Server::connect(const RTcpConnPtr& conn)
{
    contexts_.add(conn.get());
    Context& context = contexts_.getContext(conn.get());

    conn->setHighWaterMarkFunc(
        waterMark_,
        [&context](const RTcpConnPtr& conn, std::size_t size) -> void
        {
            context.isWaterMark = true;
        }
    );
    conn->setWriteCompleteFunc(
        [this, &context](const RTcpConnPtr& conn) -> void
        {
            context.isWaterMark = false;
            while (!context.isWaterMark)
                bodyFunc_(context);
        }
    );
}

void Server::disconnect(const RTcpConnPtr& conn)
{
    contexts_.release(conn.get());
}

} // namespace qinmo::net::http
