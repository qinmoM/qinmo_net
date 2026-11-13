#include "qinmo/net/http/Request.h"

namespace qinmo::net::http
{

Request::Method Request::getMethod() const
{
    return method_;
}

std::string Request::getPath() const
{
    return path_;
}

std::string Request::getQuery() const
{
    return query_;
}

Request::Version Request::getVersion() const
{
    return version_;
}


void Request::setMethod(Request::Method method)
{
    method_ = method;
}

void Request::setPath(qinmo::StringView path)
{
    path_ = std::string(path.data(), path.size());
}

void Request::setQuery(qinmo::StringView query)
{
    query_ = std::string(query.data(), query.size());
}

void Request::setVersion(Request::Version version)
{
    version_ = version;
}


Headers& Request::getHeaders()
{
    return headers_;
}


void Request::setBody(qinmo::StringView body)
{
    body_ = std::string(body.data(), body.size());
}

qinmo::StringView Request::getBody() const
{
    return qinmo::StringView(body_.data(), body_.size());
}

} // namespace qinmo::net::http
