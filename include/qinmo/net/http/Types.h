#pragma once

namespace qinmo::net::http
{

enum class Version
{
    Unknown,
    Http10,
    Http11
};


enum class StateCode
{
    Unknown,
    Code200,
    Code400,
    Code404
};


enum class Method
{
    Unknown,
    Get,
    Post,
    Head
};



enum class BodyFrame
{
    None,
    Length,
    Chunked
};

} // namespace qinmo::net::http