#pragma once

#include "qinmo/net/ReactorTcpServer.h"
#include "qinmo/net/http/Request.h"
#include "qinmo/net/http/Response.h"

namespace qinmo::net
{

class PacketBuffer;
class Server;


namespace http
{


class Context;

using ContextFunc = std::function<void(Context&)>;
using MessageFunc = std::function<void(Context&, qinmo::StringView)>;



class Context
{
public:
    enum class ContextState
    {
        RequestLine,
        Headers,
        Body,
        Complete
    };

    enum class ServerHandle
    {
        None,
        Shutdown,
        Close,
        Reset
    };

    friend class Server;

private:
    ServerHandle parse(qinmo::net::PacketBuffer& buffer, ContextFunc& headers, MessageFunc& message, ContextFunc& complete);
    // bool isParsed() const;

    void reset();

private:
    /// @return false if parsing error
    /// @note strict matching
    bool parseRequestLine(qinmo::StringView sv);
    bool parseHeader(qinmo::StringView sv);

    /// @brief verify whether the headers is valid
    bool verifyHeader();

    Request request_;
    ContextState state_ = ContextState::RequestLine;
    std::size_t length_ = 0;
    std::size_t currLength_ = 0;


    bool isWaterMark = false;
};
} // namespace http
} // namespace qinmo::net
