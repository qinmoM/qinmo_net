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



void Server::handleConnect(const RTcpConnPtr& conn, Context& context)
{
    Context::ServerHandle handle = context.handle_;
    QINMO_DEBUG("handle connect. handleId=", static_cast<int>(handle));

    if (Context::ServerHandle::Close == handle)
        conn->close();
    else if (Context::ServerHandle::Shutdown == handle)
        conn->shutdown();
    else if (Context::ServerHandle::Reset == handle)
        context.reset();
}



void Server::onMessage(const RTcpConnPtr& conn, PacketBuffer& buffer, Timestamp timestamp)
{
    Context& context = contexts_.getContext(conn);

    QINMO_DEBUG("enter parse in http server");
    context.parse(buffer, headersFunc_, messageFunc_, completeFunc_);
    handleConnect(conn, context);
    QINMO_DEBUG("quit parse in http server.");
}

void Server::connect(const RTcpConnPtr& conn)
{
    contexts_.add(conn);
    Context& context = contexts_.getContext(conn);

    // set event function if exists response-func
    if (bodyFunc_)
    {
        conn->setHighWaterMarkFunc(
            waterMark_,
            [&context](const RTcpConnPtr&, std::size_t) -> void
            {
                context.isWaterMark_ = true;
            });
        conn->setWriteCompleteFunc(
            [this, &context](const RTcpConnPtr& conn) -> void
            {
                QINMO_DEBUG("enter Response func, fd=", conn->getfd());
                context.isWaterMark_ = false;

                while (!context.isWaterMark_
                    && Response::State::HeadersSent == context.response_.state_
                    && Request::ParseState::Error != context.request_.state_)
                {
                    bodyFunc_(context);
                    handleConnect(conn, context);
                }

                QINMO_DEBUG("quit Response func, fd=", conn->getfd());
            });
    }
}

void Server::disconnect(const RTcpConnPtr& conn)
{
    contexts_.release(conn);
}

} // namespace qinmo::net::http
