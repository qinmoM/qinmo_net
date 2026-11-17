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
        Complete,
        Error
    };


    Method method_ = Method::Unknown;
    std::string path_ = "";
    std::string query_ = "";
    Version version_ = Version::Unknown;

    Headers headers_;
    std::string body_;

    BodyFrame frame_ = BodyFrame::None;
    ParseState state_ = ParseState::RequestLine;
    std::size_t length_ = 0;
    std::size_t currLength_ = 0;
    std::size_t targetCount_ = 0;
    std::size_t currCount_ = 0;
    bool isDigit_ = true;
};
} // namespace qinmo::net::http
