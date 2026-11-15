#include "qinmo/net/http/Context.h"
#include "qinmo/net/PacketBuffer.h"
#include "qinmo/tool/Logger.h"

namespace qinmo::net::http
{

// Context::Context()
// { }

// Context::~Context()
// {
//     ;
// }



Context::ServerHandle Context::parse(qinmo::net::PacketBuffer& buffer, ContextFunc& headers, MessageFunc& message, ContextFunc& complete)
{
    ServerHandle handle = ServerHandle::None;
    while (true)
    {
        if (ContextState::RequestLine == state_)
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
            state_ = ContextState::Headers;
        }
        else if (ContextState::Headers == state_)
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

                state_ = ContextState::Body;

                if (headers)
                    headers(*this);
            }
        }
        else if (ContextState::Body == state_)
        {
            // obtain the current buffer data
            qinmo::StringView sv;
            buffer.peekAll(sv);
            std::size_t num = std::min(sv.size(), length_ - currLength_);

            std::string s;
            buffer.retrieveString(num, s);
            request_.setBody(qinmo::StringView(s.data(), s.size()));
            state_ = ContextState::Complete;
            break;
        }
    }
    return handle;
}

void Context::reset()
{
    request_ = Request{};
    state_ = ContextState::RequestLine;
    length_ = 0;
}



bool Context::parseRequestLine(qinmo::StringView sv)
{
    Request::Method method = Request::Method::Unknown;
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
        method = Request::Method::Get;
    else if ("POST" == temp)
        method = Request::Method::Post;
    else if ("HEAD" == temp)
        method = Request::Method::Head;

    if (method == Request::Method::Unknown)
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

    request_.setMethod(method);
    request_.setPath(path);
    request_.setQuery(query);
    request_.setVersion(version);
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

    request_.getHeaders().add(name, value);
    return true;
}



bool Context::verifyHeader()
{
    Headers& headers = request_.getHeaders();

    qinmo::StringView sv;
    bool hasLength = headers.has("context-length");
    if (!hasLength)
        return true;

    if (!headers.getSingle("context-length", sv))
        return false;

    length_ = std::stoul(std::string(sv.data(), sv.size()));
    // currLength_ = 0;
    return true;
}
} // namespace qinmo::net::http
