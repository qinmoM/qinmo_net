#pragma once

#include "qinmo/net/http/Context.h"
#include <unordered_map>

namespace qinmo::net
{

namespace http
{

class ContextsManager
{
public:
    using TcpConnect = qinmo::net::ReactorTcpConnection;
    using TcpPtr = qinmo::net::RTcpConnPtr;

public:
    void add(TcpPtr ptr, bool isSaveBody = true);
    void release(TcpPtr ptr);

    Context& getContext(TcpPtr ptr);

private:
    static thread_local std::unordered_map<TcpConnect*, Context> contexts_;
};
} // namespace http
} // namespace qinmo::net
