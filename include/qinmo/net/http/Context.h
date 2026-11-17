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

    friend class Server;

public:
/*
        context-related
*/
    /// @brief close this connection
    void conClose();


/*
        request-related
*/
    Method reqMethod() const;
    Version reqVersion() const;
    /// @return return true if the `name` existed
    bool reqHasHeader(qinmo::StringView name) const;
    /// @param nameToValue input the field `name`, output the field `value` if return true
    /// @return return true if the field name exists and occurs once only
    bool reqSingleHeader(qinmo::StringView& nameToValue) const;
    /// @return all of value by 'name' parameter
    std::vector<qinmo::StringView> reqAllHeaders(qinmo::StringView name) const;
    qinmo::StringView reqBody() const;


/*
        response-related
*/
    /// @note only be used before calling `resSend()`
    bool resSetVersion(Version version);
    /// @note only be used before calling `resSend()`
    bool resSetStateCode(StateCode stateCode);
    /// @note only be used before calling `resSend()`
    bool resSetStateMessage(qinmo::StringView sv);
    /// @note only be used before calling `resSend()`
    /// @note the following names are disallowed:
    /// @note - `Content-Length`/`Transfer-Encoding` : use `resSetLength()`/`resSetChunked()` instead
    /// @note - `Connection` : use `resSetKeepAlive()` instead
    /// @return return false if the  name or value contains invalid characters  or  any disallowed name
    bool resSetHeader(qinmo::StringView name, qinmo::StringView value);

    /// @brief send part-body
    /// @note the first call, implicitly sends response header
    /// @note auto-assemble CRTL and size in chunked mode
    /// @note do not send 0-byte data, Use `resEnd()` to send the terminator in chunked mode
    /// @note only be used before calling `resEnd()`
    bool resSend(qinmo::StringView sv);
    /// @brief used to send the terminator in chunked mode; check whether the length is sufficient in length mode
    /// @note the first call without call `resSend()`, implicitly sends response header
    /// @note must call this to terminate even in the length mode
    bool resEnd();

    /// @brief change to `Content-Length` mode
    /// @param num length of all data
    /// @note only call this before `resSend()`
    /// @note auto-write `Content-Length` to header
    bool resSetLength(std::size_t num);
    /// @brief change to `Transfer-Encoding:chunked` mode
    /// @note only call this before `resSend()`
    /// @note auto-write `Transfer-Encoding: chunked` to response header
    bool resSetChunked();
    /// @brief determine whether keep-alive
    /// @note auto-write `Connection` to response header
    bool resSetKeepAlive(bool isKeepAlive = true);

public:
    Context(qinmo::net::RTcpConnPtr conn, bool isSaveBody = true);

private:
    void parse(qinmo::net::PacketBuffer& buffer, ContextFunc& headers, MessageFunc& message, ContextFunc& complete);

    void reset();

private:
    void serHandle(ServerHandle handle);
    bool responseError();
    void sendResponseHeaderByMode();
    void fillDefault();
    void sendBodyByMode(qinmo::StringView sv);
    std::string formatHeader() const;

    bool isValid(Method method) const;
    bool isValid(Version version) const;
    bool isValid(StateCode stateCode) const;
    bool isValidFieldName(qinmo::StringView sv) const;
    bool isValidFieldValue(qinmo::StringView sv) const;

    /// @return false if parsing error
    /// @note strict matching
    bool parseRequestLine(qinmo::StringView sv);
    bool parseHeader(qinmo::StringView sv);

    /// @brief verify whether the request headers is valid
    bool verifyRequestHeader();

    Request request_;
    Response response_;
    qinmo::net::RTcpConnPtr conn_;

    ServerHandle handle_ = ServerHandle::None;
    const bool isSaveBody_;
    bool isWaterMark_ = false;
};
} // namespace http
} // namespace qinmo::net
