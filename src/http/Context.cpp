#include "qinmo/net/http/Context.h"
#include "qinmo/net/PacketBuffer.h"
#include "qinmo/tool/Logger.h"

namespace qinmo::net::http
{


void Context::conClose()
{
    serHandle(ServerHandle::Close);
}



Method Context::reqMethod() const
{
    return request_.method_;
}

Version Context::reqVersion() const
{
    return request_.version_;
}

bool Context::reqHasHeader(qinmo::StringView name) const
{
    return request_.headers_.has(name);
}

bool Context::reqSingleHeader(qinmo::StringView& nameToValue) const
{
    return request_.headers_.getSingle(nameToValue, nameToValue);
}

std::vector<qinmo::StringView> Context::reqAllHeaders(qinmo::StringView name) const
{
    return request_.headers_.getAll(name);
}

qinmo::StringView Context::reqBody() const
{
    return qinmo::StringView(request_.body_.data(), request_.body_.size());
}



bool Context::resSetVersion(Version version)
{
    if (Response::State::Init != response_.state_ || !isValid(version))
        return false;

    response_.version_ = version;
    return true;
}

bool Context::resSetStateCode(StateCode stateCode)
{
    if (Response::State::Init != response_.state_ || !isValid(stateCode))
        return false;

    response_.stateCode_ = stateCode;
    return true;
}

bool Context::resSetStateMessage(qinmo::StringView sv)
{
    if (Response::State::Init != response_.state_)
        return false;

    response_.stateMessage_ = std::string(sv.data(), sv.size());
    return true;
}

bool Context::resSetHeader(qinmo::StringView name, qinmo::StringView value)
{
    if (Response::State::Init != response_.state_ || !isValidFieldName(name) || !isValidFieldValue(value))
        return false;

    std::string n(name.data(), name.size());
    for (char& c : n)
        c = qinmo::toLower(c);

    if (n == "connection" || n == "transfer-encoding" || n == "content-length")
        return false;

    response_.headers_.add(name, value);
    return true;
}


bool Context::resSend(qinmo::StringView sv)
{
    if (Response::State::Complete == response_.state_
        || Response::State::Error == response_.state_
        || sv.empty())
        return false;

    if (Response::State::HeadersSent == response_.state_)
    {
        if (BodyFrame::Length == response_.frame_)
        {
            // check whether the length is greater than specified value
            if (sv.size() > response_.length_ - response_.currLength_)
                return responseError();

            // send body-pair
            sendBodyByMode(sv);

            // modify state
            response_.currLength_ += sv.size();
            if (response_.currLength_ == response_.length_)
                response_.state_ = Response::State::Complete;
        }
        else if (BodyFrame::Chunked == response_.frame_)
        {
            // send response body-pair
            sendBodyByMode(sv);
        }
    }
    else if (Response::State::Init == response_.state_)
    {
        // fill default values
        fillDefault();

        if (BodyFrame::Length == response_.frame_)
        {
            // check whether it exceeds the specified byte-count
            if (sv.size() > response_.length_)
                return responseError();

            // build and send response header
            sendResponseHeaderByMode();

            // send response body-pair
            sendBodyByMode(sv);

            // modify state
            response_.state_ = Response::State::HeadersSent;
            response_.currLength_ += sv.size();
        }
        else if (BodyFrame::Chunked == response_.frame_)
        {
            // build and send response header
            sendResponseHeaderByMode();

            // send response body-pair
            sendBodyByMode(sv);

            // modify state
            response_.state_ = Response::State::HeadersSent;
        }
    }

    return true;
}

bool Context::resEnd()
{
    if (Response::State::Error == response_.state_
        || Response::State::Complete == response_.state_)
        return false;

    if (Response::State::Init == response_.state_)
    {
        if (BodyFrame::Length == response_.frame_)
        {
            // check whether the length is not equal to specified value
            if (response_.currLength_ != response_.length_)
                return responseError();

            // build and send response header
            sendResponseHeaderByMode();
        }
        else if (BodyFrame::Chunked == response_.frame_)
        {
            // build and send response header
            sendResponseHeaderByMode();

            // send terminator to client
            sendBodyByMode(qinmo::StringView());
        }
    }
    else if (Response::State::HeadersSent == response_.state_)
    {
        if (BodyFrame::Length == response_.frame_)
        {
            // check whether the length is not equal to specified value
            if (response_.currLength_ != response_.length_)
                return responseError();
        }
        else if (BodyFrame::Chunked == response_.frame_)
        {
            // send terminator to client
            sendBodyByMode(qinmo::StringView());
        }
    }

    // modify state
    response_.state_ = Response::State::Complete;
    if (Request::ParseState::Complete == request_.state_)
    {
        if (response_.isKeepAlive_)
            serHandle(ServerHandle::Reset);
        else
            serHandle(ServerHandle::Shutdown);
    }

    return true;
}

bool Context::resSetLength(std::size_t num)
{
    if (Response::State::Init != response_.state_)
        return false;

    response_.frame_ = BodyFrame::Length;
    response_.length_ = num;
    return true;
}

bool Context::resSetChunked()
{
    if (Response::State::Init != response_.state_)
        return false;

    response_.frame_ = BodyFrame::Chunked;
    return true;
}

bool Context::resSetKeepAlive(bool isKeepAlive)
{
    if (Response::State::Init != response_.state_)
        return false;

    response_.isKeepAlive_ = isKeepAlive;
    return true;
}



Context::Context(qinmo::net::RTcpConnPtr conn, bool isSaveBody)
    : conn_(conn)
    , isSaveBody_(isSaveBody)
{ }



void Context::parse(qinmo::net::PacketBuffer& buffer, ContextFunc& headers, MessageFunc& message, ContextFunc& complete)
{
    ServerHandle handle = ServerHandle::None;
    while (true)
    {
        if (Request::ParseState::RequestLine == request_.state_)
        {
            // obtain the current buffer data
            qinmo::StringView sv;
            buffer.peekAll(sv);

            // check whether the end of request line has been reached
            std::size_t pos = sv.find("\r\n");
            if (qinmo::StringView::npos == pos)
                break;

            // parse failed
            if (!parseRequestLine(sv.subview(0, pos)))
            {
                request_.state_ = Request::ParseState::Error;
                handle = ServerHandle::Close;
                QINMO_DEBUG("Parse request-line failed: ", sv);
                break;
            }

            // parse successful
            QINMO_DEBUG("Parse request-line successful: method:", static_cast<int>(request_.method_),
                " path-query:", request_.path_, '-', request_.query_,
                " version:", static_cast<int>(request_.version_));
            std::string s;
            buffer.retrieveString(pos + 2, s);
            request_.state_ = Request::ParseState::Headers;
        }
        else if (Request::ParseState::Headers == request_.state_)
        {
            // obtain the current buffer data
            qinmo::StringView sv;
            buffer.peekAll(sv);

            // check whether the end of request line has been reached
            std::size_t pos = sv.find("\r\n");
            if (qinmo::StringView::npos == pos)
                break;

            // parse headers
            if (0 != pos && !parseHeader(sv.subview(0, pos)))
            {
                request_.state_ = Request::ParseState::Error;
                handle = ServerHandle::Close;
                QINMO_DEBUG("Parse Header failed: ", sv);
                break;
            }

            // parse successful
            QINMO_DEBUG("Rarse Header successful: ", sv);
            std::string s;
            buffer.retrieveString(pos + 2, s);

            // end of header
            if (0 == pos)
            {
                if (!verifyRequestHeader())
                {
                    handle = ServerHandle::Close;
                    break;
                }

                request_.state_ = Request::ParseState::Body;

                if (headers)
                    headers(*this);

                if (BodyFrame::None == request_.frame_
                    || (BodyFrame::Length == request_.frame_ && 0 == request_.length_))
                {
                    request_.state_ = Request::ParseState::Complete;

                    if (complete)
                        complete(*this);
                }
            }
        }
        else if (Request::ParseState::Body == request_.state_)
        {
            // obtain the current buffer data
            qinmo::StringView sv;
            buffer.peekAll(sv);

            if (BodyFrame::Length == request_.frame_)
            {
                std::size_t num = std::min(sv.size(), request_.length_ - request_.currLength_);
                std::string s;
                buffer.retrieveString(num, s);

                // modify state
                request_.currLength_ += num;
                if (isSaveBody_)
                    request_.body_.append(s);

                if (message)
                    message(*this, qinmo::StringView(s.data(), s.size()));

                // complete body
                if (request_.currLength_ == request_.length_)
                {
                    request_.state_ = Request::ParseState::Complete;

                    if (complete)
                        complete(*this);
                }
            }
            else if (BodyFrame::Chunked == request_.frame_)
            {
                if (request_.isDigit_)
                {
                    std::size_t pos = sv.find("\r\n");
                    if (qinmo::StringView::npos == pos)
                        break;

                    if (0 == pos)
                    {
                        handle = ServerHandle::Close;
                        break;
                    }

                    std::size_t num = 0;
                    try { num = std::stoul(std::string(sv.data(), pos), nullptr, 16); }
                    catch(...)
                    {
                        request_.state_ = Request::ParseState::Error;
                        serHandle(ServerHandle::Close);
                        QINMO_DEBUG("parse length failed");
                        break;
                    }
                    std::string s;
                    buffer.retrieveString(pos + 2, s);
                    request_.targetCount_ = num;
                    request_.isDigit_ = false;
                }
                else
                {
                    if (request_.targetCount_ == request_.currCount_)
                    {
                        if (sv.size() < 2)
                            break;

                        std::string s;
                        buffer.retrieveString(2, s);
                        if ("\r\n" != s)
                        {
                            request_.state_ = Request::ParseState::Error;
                            serHandle(ServerHandle::Close);
                            QINMO_DEBUG("parse body failed");
                            break;
                        }

                        if (0 == request_.targetCount_)
                        {
                            request_.state_ = Request::ParseState::Complete;

                            if (complete)
                                complete(*this);
                        }

                        request_.targetCount_ = 0;
                        request_.currCount_ = 0;
                        request_.isDigit_ = true;
                    }
                    else
                    {
                        std::size_t num = std::min(sv.size(), request_.targetCount_ - request_.currCount_);
                        std::string s;
                        buffer.retrieveString(num, s);

                        request_.currCount_ += num;
                        if (isSaveBody_)
                            request_.body_.append(s);

                        if (message)
                            message(*this, qinmo::StringView(s.data(), s.size()));
                    }
                }
            }
        }
        else
        {
            break;
        }
    }
    serHandle(handle);
}

void Context::reset()
{
    QINMO_DEBUG("reset in Context: fd:", conn_->getfd());
    request_ = Request{};
    response_ = Response{};
    isWaterMark_ = false;
    handle_ = ServerHandle::None;
}



void Context::serHandle(ServerHandle handle)
{
    if (ServerHandle::Close == handle_ || ServerHandle::None == handle || handle_ == handle)
        return;

    if (ServerHandle::None == handle_ || ServerHandle::Close == handle)
    {
        handle_ = handle;
        return;
    }

    if ((ServerHandle::Reset == handle && ServerHandle::Shutdown == handle_)
        || (ServerHandle::Shutdown == handle && ServerHandle::Reset == handle_))
    {
        QINMO_ERROR("reset and shutdown cannot be called together");
    }
}

bool Context::responseError()
{
    serHandle(ServerHandle::Close);
    response_.state_ = Response::State::Error;
    return false;
}

void Context::sendResponseHeaderByMode()
{
    if (BodyFrame::None == response_.frame_)
    {
        QINMO_ERROR("must not be BodyFrame::None when send header.");
        return;
    }

    response_.headers_.add("Connection", (response_.isKeepAlive_ ? "keep-alive" : "close"));

    if (BodyFrame::Chunked == response_.frame_)
        response_.headers_.add("Transfer-Encoding", "chunked");
    else if (BodyFrame::Length == response_.frame_)
        response_.headers_.add("Content-Length", std::to_string(response_.length_));

    conn_->send(formatHeader());
}

void Context::fillDefault()
{
    if (!isValid(response_.version_))
        response_.version_ = request_.version_;

    if (!isValid(response_.stateCode_))
        response_.stateCode_ = StateCode::Code200;

    if (BodyFrame::None == response_.frame_)
        response_.frame_ = BodyFrame::Chunked;
}

void Context::sendBodyByMode(qinmo::StringView sv)
{
    if (BodyFrame::None == response_.frame_)
    {
        QINMO_ERROR("must not be BodyFrame::None when send body.");
        return;
    }

    if (BodyFrame::Chunked == response_.frame_)
    {
        std::string str;
        if (sv.empty())
        {
            str = "0";
        }
        else
        {
            std::ostringstream oss;
            oss << std::hex << sv.size() << "\r\n";
            str = oss.str();
        }
        str += (std::string(sv.data(), sv.size()) + "\r\n");
        conn_->send(str);
    }
    else if (BodyFrame::Length == response_.frame_)
    {
        std::string str(sv.data(), sv.size());
        conn_->send(str);
    }
}

std::string Context::formatHeader() const
{
    std::string res;
    res.reserve(512);

    switch (response_.version_)
    {
        case Version::Http10: res += "HTTP/1.0"; break;
        case Version::Http11: res += "HTTP/1.1"; break;
        default:
            QINMO_ERROR("Unknown version info");
    }
    res += ' ';

    switch (response_.stateCode_)
    {
        case StateCode::Code200: res += "200"; break;
        case StateCode::Code400: res += "400"; break;
        case StateCode::Code404: res += "404"; break;
        default:
            QINMO_ERROR("Unknown version info");
    }
    res += ' ';

    res = res + response_.stateMessage_ + "\r\n";

    const Headers::HeadersType& headers = response_.headers_.headers_;
    for (std::size_t i = 0; i < headers.size(); ++i)
        res = res + headers[i].first + ": " + headers[i].second + "\r\n";

    res += "\r\n";
    return res;
}

bool Context::isValid(Method method) const
{
    switch (method)
    {
        case Method::Get:
        case Method::Head:
        case Method::Post:
            return true;
        default:
            return false;
    }
}

bool Context::isValid(Version version) const
{
    switch (version)
    {
        case Version::Http10:
        case Version::Http11:
            return true;
        default:
            return false;
    }
}

bool Context::isValid(StateCode stateCode) const
{
    switch (stateCode)
    {
        case StateCode::Code200:
        case StateCode::Code400:
        case StateCode::Code404:
            return true;
        default:
            return false;
    }
}

bool Context::isValidFieldName(qinmo::StringView sv) const
{
    for (const char c : sv)
    {
        if (!Headers::isTokenChar(c))
            return false;
    }

    return true;
}

bool Context::isValidFieldValue(qinmo::StringView sv) const
{
    for (const char c : sv)
    {
        if (!Headers::isValidValue(static_cast<unsigned char>(c)))
            return false;
    }

    return true;
}




bool Context::parseRequestLine(qinmo::StringView sv)
{
    Method method = Method::Unknown;
    qinmo::StringView path = "";
    qinmo::StringView query = "";
    Version version = Version::Unknown;
    qinmo::StringView temp;

    std::size_t pos = sv.find(' ');
    if (qinmo::StringView::npos == pos || 0 == pos)
        return false;

    temp = sv.subview(0, pos);
    sv.remove_prefix(pos + 1);
    if (!sv.empty() && ' ' == sv[0])
        return false;

    // parse method
    if ("GET" == temp)
        method = Method::Get;
    else if ("POST" == temp)
        method = Method::Post;
    else if ("HEAD" == temp)
        method = Method::Head;

    if (method == Method::Unknown)
        return false;


    pos = sv.find(' ');
    if (qinmo::StringView::npos == pos || 0 == pos)
        return false;

    temp = sv.subview(0, pos);
    sv.remove_prefix(pos + 1);

    // parse path and query
    std::size_t decollator = temp.find('?');
    if (qinmo::StringView::npos == decollator)
    {
        path = temp;
    }
    else
    {
        path = qinmo::StringView(temp.data(), decollator);
        query = qinmo::StringView(temp.data() + decollator + 1, temp.size() - decollator - 1);
    }


    pos = sv.find(' ');
    if (qinmo::StringView::npos != pos || 8 != sv.size())
        return false;

    temp = sv;

    // parse version
    if ("HTTP/1.0" == temp)
        version = Version::Http10;
    else if ("HTTP/1.1" == temp)
        version = Version::Http11;

    if (version == Version::Unknown)
        return false;

    request_.method_ = method;
    request_.path_ = std::string(path.data(), path.size());
    request_.query_ = std::string(query.data(), query.size());
    request_.version_ = version;
    return true;
}

bool Context::parseHeader(qinmo::StringView sv)
{
    std::size_t pos = sv.find(':');
    if (qinmo::StringView::npos == pos || 0 == pos)
        return false;

    qinmo::StringView name = sv.subview(0, pos);
    qinmo::StringView value = sv.subview(pos + 1);

    // -- name --
    if (!isValidFieldName(name))
        return false;

    // -- value --
    pos = value.find_first_not_of(" \t");
    if (qinmo::StringView::npos == pos)
        pos = value.size();

    value.remove_prefix(pos);

    if (!value.empty())
    {
        pos = value.find_last_not_of(" \t");
        value.remove_suffix(value.size() - pos - 1);
    }

    if (!isValidFieldValue(value))
        return false;

    request_.headers_.add(name, value);
    return true;
}



bool Context::verifyRequestHeader()
{
    Headers& headers = request_.headers_;

    qinmo::StringView sv;
    bool hasLength = headers.has("content-length");
    bool hasChunked = headers.has("transfer-encoding");
    if (hasChunked && hasLength)
        return false;

    if (hasChunked)
    {
        if (!headers.getSingle("transfer-encoding", sv))
            return false;

        request_.frame_ = BodyFrame::Chunked;
    }
    else if (hasLength)
    {
        if (!headers.getSingle("content-length", sv))
            return false;

        request_.frame_ = BodyFrame::Length;
        try { request_.length_ = std::stoul(std::string(sv.data(), sv.size())); }
        catch(...) { return false; }
    }

    return true;
}
} // namespace qinmo::net::http
