#include "qinmo/net/http/Context.h"
#include "qinmo/net/PacketBuffer.h"
#include "qinmo/tool/Logger.h"

namespace qinmo::net::http
{


Method Context::reqMethod() const
{
    return request_.method_;
}

Version Context::reqVersion() const
{
    return request_.version_;
}

std::vector<qinmo::StringView> Context::reqHeader(qinmo::StringView name) const
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
    if (Response::State::Init != response_.state_)
        return false;

    response_.headers_.add(name, value);
    return true;
}


bool Context::resSend(qinmo::StringView sv)
{
    if (Response::State::Complete == response_.state_
        || Response::State::Error == response_.state_)
        return false;

    if (Response::State::HeadersSent == response_.state_)
    {
        if (sv.size() > response_.length_)
        {
            serHandle(ServerHandle::Close);
            response_.state_ = Response::State::Error;
            return false;
        }

        std::string str(sv.data(), sv.size());
        conn_->send(str);

        response_.currLength_ += sv.size();
        if (response_.currLength_ == response_.length_)
            response_.state_ = Response::State::Complete;
    }
    else if (Response::State::Init == response_.state_)
    {
        if (!isValid(response_.version_))
            response_.version_ = request_.version_;
        if (!isValid(response_.stateCode_))
            response_.stateCode_ = StateCode::Code200;

        if (sv.size() > response_.length_)
        {
            serHandle(ServerHandle::Close);
            response_.state_ = Response::State::Error;
            return false;
        }

        response_.headers_.add("Content-Length", std::to_string(response_.length_));
        response_.headers_.add("Connection", (response_.isKeepAlive_ ? "keep-alive" : "close"));
        conn_->send(formatHeader());

        std::string str(sv.data(), sv.size());
        conn_->send(str);
        response_.state_ = Response::State::HeadersSent;

        response_.currLength_ += sv.size();
        if (response_.currLength_ == response_.length_)
            response_.state_ = Response::State::Complete;
    }
}

bool Context::resEnd()
{
    // if ()
    // {
    //     ;
    // }
}

bool Context::resSetLength(std::size_t num)
{
    if (Response::State::Init != response_.state_)
        return false;

    response_.length_ = num;
    return true;
}

bool Context::resSetKeepAlive(bool isKeepAlive)
{
    if (Response::State::Init != response_.state_)
        return false;

    response_.isKeepAlive_ = isKeepAlive;
    return true;
}



// Context::Context()
// { }

// Context::~Context()
// {
//     ;
// }



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
                handle = ServerHandle::Close;
                QINMO_DEBUG("Parse request-line failed: ", sv);
                break;
            }

            // parse successful
            QINMO_DEBUG("Parse request-line successful: ", sv);
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
                QINMO_DEBUG("Parse Header failed: ", sv);
                handle = ServerHandle::Close;
                break;
            }

            // parse successful
            QINMO_DEBUG("Rarse Header successful: ", sv);
            std::string s;
            buffer.retrieveString(pos + 2, s);

            // end of header
            if (0 == pos)
            {
                if (!verifyHeader())
                {
                    handle = ServerHandle::Close;
                    break;
                }

                request_.state_ = Request::ParseState::Body;

                if (headers)
                    headers(*this);
            }
        }
        else if (Request::ParseState::Body == request_.state_)
        {
            // obtain the current buffer data
            qinmo::StringView sv;
            buffer.peekAll(sv);
            std::size_t num = std::min(sv.size(), request_.length_ - request_.currLength_);

            std::string s;
            buffer.retrieveString(num, s);
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
        else
        {
            break;
        }
    }
    serHandle(handle);
    // return handle;
}

void Context::reset()
{
    request_ = Request{};
    response_ = Response{};
    isWaterMark_ = false;
    bodyFrame_ = BodyFrame::None;
    handle_ = ServerHandle::None;
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

    request_.method_;
    request_.path_;
    request_.query_;
    request_.version_;
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
    for (const char c : name)
    {
        if (!Headers::isTokenChar(c))
            return false;
    }

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

    for (const char c : value)
    {
        if (!Headers::isValidValue(static_cast<unsigned char>(c)))
            return false;
    }

    request_.headers_.add(name, value);
    return true;
}



bool Context::verifyHeader()
{
    Headers& headers = request_.headers_;

    qinmo::StringView sv;
    bool hasLength = headers.has("context-length");
    if (!hasLength)
        return true;

    if (!headers.getSingle("context-length", sv))
        return false;

    request_.length_ = std::stoul(std::string(sv.data(), sv.size()));
    // currLength_ = 0;
    return true;
}
} // namespace qinmo::net::http
