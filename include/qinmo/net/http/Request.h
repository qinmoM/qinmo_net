#pragma once

#include "qinmo/net/http/Headers.h"

namespace qinmo::net::http
{

class Request
{
public:
    enum class Method
    {
        Unknown,
        Get,
        Post,
        Head
    };

    enum class Version
    {
        Unknown,
        Http10,
        Http11
    };


    static bool isTokenChar(char c);

public:
    Method getMethod() const;
    std::string getPath() const;
    std::string getQuery() const;
    Version getVersion() const;

    void setMethod(Method method);
    void setPath(qinmo::StringView path);
    void setQuery(qinmo::StringView query);
    void setVersion(Version version);

    Headers& getHeaders();

private:
    Method method_ = Method::Unknown;
    std::string path_ = "";
    std::string query_ = "";
    Version version_ = Version::Unknown;

    Headers headers_;

};
} // namespace qinmo::net::http
