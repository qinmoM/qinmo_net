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
    enum class ServerHandle
    {
        None,
        Shutdown,
        Close,
        Reset
    };

    enum class BodyFrame
    {
        None,
        Length,
        Chunked
    };

    friend class Server;

public:
/*
        request-related
*/
    Method reqMethod() const;
    Version reqVersion() const;
    std::vector<qinmo::StringView> reqHeader(qinmo::StringView name) const;
    qinmo::StringView reqBody() const;


/*
        response-related
*/
    bool resSetVersion(Version version);
    bool resSetStateCode(StateCode stateCode);
    bool resSetStateMessage(qinmo::StringView sv);
    bool resSetHeader(qinmo::StringView name, qinmo::StringView value);

    bool resSend(qinmo::StringView sv);
    bool resEnd();

public:
    Context(qinmo::net::RTcpConnPtr conn, bool isSaveBody = true);

private:
    ServerHandle parse(qinmo::net::PacketBuffer& buffer, ContextFunc& headers, MessageFunc& message, ContextFunc& complete);
    // bool isParsed() const;

    void reset();

private:
    bool isValid(Method method) const;
    bool isValid(Version version) const;
    bool isValid(StateCode stateCode) const;

    /// @return false if parsing error
    /// @note strict matching
    bool parseRequestLine(qinmo::StringView sv);
    bool parseHeader(qinmo::StringView sv);

    /// @brief verify whether the headers is valid
    bool verifyHeader();

    Request request_;
    Response response_;
    qinmo::net::RTcpConnPtr conn_;

    BodyFrame bodyFrame_ = BodyFrame::None;
    const bool isSaveBody_;
    bool isWaterMark_ = false;
};
} // namespace http
} // namespace qinmo::net
