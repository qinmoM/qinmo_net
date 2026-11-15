#pragma once

#include "qinmo/net/http/Types.h"
#include "qinmo/net/http/Headers.h"

namespace qinmo::net::http
{

struct Request
{
    enum class ParseState
    {
        RequestLine,
        Headers,
        Body,
        Complete
    };


    Method method_ = Method::Unknown;
    std::string path_ = "";
    std::string query_ = "";
    Version version_ = Version::Unknown;

    Headers headers_;
    std::string body_;

    ParseState state_ = ParseState::RequestLine;
    std::size_t length_ = 0;
    std::size_t currLength_ = 0;
};
} // namespace qinmo::net::http
