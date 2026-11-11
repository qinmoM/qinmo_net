#include "qinmo/net/http/Context.h"
#include "qinmo/net/PacketBuffer.h"

namespace qinmo::net::http
{

Context::Context()
{
    ;
}

Context::~Context()
{
    ;
}



bool Context::parse(qinmo::net::PacketBuffer& buffer)
{
    int noError = true;
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
                noError = false;
                break;
            }

            // parse successful
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
                noError = false;
                break;
            }

            // parse successful
            std::string s;
            buffer.retrieveString(pos + 2, s);

            // end of header
            if (0 == pos)
            {
                /// valid
                state_ = ContextState::Body;
            }
        }
        else if (ContextState::Body == state_)
        {
            ;
        }
    }
    return noError;
}



bool Context::parseRequestLine(qinmo::StringView sv)
{
    Request::Method method = Request::Method::Unknown;
    qinmo::StringView path = "";
    qinmo::StringView query = "";
    Request::Version version = Request::Version::Unknown;
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
        version = Request::Version::Http10;
    else if ("HTTP/1.1" == temp)
        version = Request::Version::Http11;

    if (version == Request::Version::Unknown)
        return false;

    request_.setMethod(method);
    request_.setPath(path);
    request_.setQuery(query);
    request_.setVersion(version);
    return true;
}

// {
    // std::size_t nullpos = qinmo::StringView::npos;
    // // locate method to `temp`
    // qinmo::StringView temp;
    // std::size_t pos = sv.find_first_not_of(' ');
    // if (pos == nullpos)
    //     return false;
    //
    // sv = sv.subview(pos);
    // pos = sv.find(' ');
    // if (pos == nullpos)
    //     return false;
    //
    // temp = sv.subview(0, pos);
    //
    // // locate path in `sv`
    // sv = sv.subview(pos);
    // pos = sv.find_first_not_of(' ');
    // if (pos == nullpos)
    //     return false;
    //
    // sv = sv.subview(pos);
    //
    // // parse method
    // if ("GET" == temp)
    //     request_.setMethod(Request::Method::Get);
    // else if ("POST" == temp)
    //     request_.setMethod(Request::Method::Post);
    // else if ("HEAD" == temp)
    //     request_.setMethod(Request::Method::Head);
    // else
    //     request_.setMethod(Request::Method::Unknown);
    //
    // if (request_.getMethod() == Request::Method::Unknown)
    //     return false;
    //
    // // locate query and path to `temp`
    // pos = sv.find(' ');
    // if (nullpos == pos)
    //     return false;
    //
    // temp = sv.subview(0, pos);
    //
    // // locate version in `sv`
    // sv = sv.subview(pos);
    // pos = sv.find_first_not_of(' ');
    // if (pos == nullpos)
    //     return false;
    //
    // sv = sv.subview(pos);
    //
    // // parse query and path
    //
    // // parse version
    // if ()
// }

bool Context::parseHeader(qinmo::StringView sv)
{
    std::size_t pos = sv.find(':');
    if (qinmo::StringView::npos == pos || 0 == pos)
        return false;

    qinmo::StringView name = sv.subview(0, pos);
    qinmo::StringView value = sv.subview(pos + 1);

    for (const char c : name)
    {
        if (!Request::isTokenChar(c))
            return false;
    }

    pos = value.find_first_not_of(" \t");
    if (qinmo::StringView::npos == pos)
        pos = value.size();

    value.remove_prefix(pos);

    if (!value.empty())
    {
        pos = value.find_last_not_of(" \t");
        value.remove_suffix(value.size() - pos - 1);
    }

    request_.getHeaders().add(name, value);
}

} // namespace qinmo::net::http
