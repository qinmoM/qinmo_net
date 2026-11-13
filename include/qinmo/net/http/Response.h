#pragma once

#include "qinmo/net/http/Version.h"
#include "qinmo/net/http/Headers.h"

namespace qinmo::net::http
{

class Response
{
public:
    enum class StateCode
    {
        Unknown,
        Code200,
        Code400,
        Code404
    };

public:
    void setVersion(Version wersion);
    void setStateCode(StateCode stateCode);
    void setStateMessage(std::string message);
    void addHeader(qinmo::StringView name, qinmo::StringView value);
    void setBody(qinmo::StringView body);

    bool format(std::string& output) const;

private:
    Version version_ = Version::Unknown;
    StateCode stateCode_ = StateCode::Unknown;
    std::string stateMessage_;

    Headers headers_;

    std::string body_;

};
} // namespace qinmo::net::http
