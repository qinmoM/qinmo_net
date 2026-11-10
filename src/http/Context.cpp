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
            /// into request
        }
        else if (ContextState::Headers == state_)
        {
            ;
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
    ;
}

} // namespace qinmo::net::http
