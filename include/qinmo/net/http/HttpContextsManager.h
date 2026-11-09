#pragma once

#include "qinmo/net/http/HttpContext.h"
#include "qinmo/net/ReactorTcpConnect.h"
#include <unordered_map>

namespace qinmo
{

namespace net
{

class HttpContextsManager
{
public:
    void add(ReactorTcpConnect* ptr);
    void release(ReactorTcpConnect* ptr);

    HttpContext& getContext(ReactorTcpConnect* ptr);

private:
    static thread_local std::unordered_map<ReactorTcpConnect*, HttpContext> contexts_;
};
} // namespace net
} // namespace qinmo
