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
        Complete
    };

    Version version_ = Version::Unknown;
    StateCode stateCode_ = StateCode::Unknown;
    std::string stateMessage_;

    Headers headers_;
    std::string body_;

    State state_ = State::Init;
};
} // namespace qinmo::net::http
