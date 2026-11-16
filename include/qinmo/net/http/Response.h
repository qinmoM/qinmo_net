#pragma once

#include "qinmo/net/http/Types.h"
#include "qinmo/net/http/Headers.h"

namespace qinmo::net::http
{

struct Response
{
    enum class State
    {
        Init,
        HeadersSent,
        Complete,
        Error
    };

    Version version_ = Version::Unknown;
    StateCode stateCode_ = StateCode::Unknown;
    std::string stateMessage_;

    Headers headers_;
    std::string body_;

    State state_ = State::Init;
    std::size_t length_ = 0;
    std::size_t currLength_ = 0;
    bool isKeepAlive_ = false;
};
} // namespace qinmo::net::http
