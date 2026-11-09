#pragma once

#include "qinmo/net/http/Context.h"
#include "qinmo/net/ReactorTcpConnection.h"
#include <unordered_map>

namespace qinmo::net::http
{

class ContextsManager
{
public:
    using TcpConnect = qinmo::net::ReactorTcpConnection;

public:
    void add(TcpConnect* ptr);
    void release(TcpConnect* ptr);

    Context& getContext(TcpConnect* ptr);

private:
    static thread_local std::unordered_map<TcpConnect*, Context> contexts_;
};
} // namespace qinmo::net::http
