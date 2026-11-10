#pragma once

#include "qinmo/net/http/Request.h"

namespace qinmo::net
{

class PacketBuffer;

namespace http
{

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

public:
    /// @return false if any parsing error
    bool parse(qinmo::net::PacketBuffer& buffer);
    bool isParsed() const;

    void reset();

private:
    /// @return false if parsing error
    bool parseRequestLine(qinmo::StringView sv);

    ContextState state_;
};
} // namespace http
} // namespace qinmo::net
