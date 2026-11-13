#include "qinmo/net/http/ContextsManager.h"
#include "qinmo/net/ReactorTcpConnection.h"
#include "qinmo/tool/Logger.h"

namespace qinmo::net::http
{

thread_local std::unordered_map<ContextsManager::TcpConnect*, Context> ContextsManager::contexts_;

void ContextsManager::add(TcpConnect* ptr)
{
    contexts_.insert({ ptr, Context{ } });
}

void ContextsManager::release(TcpConnect* ptr)
{
    contexts_.erase(ptr);
}

Context& ContextsManager::getContext(TcpConnect* ptr)
{
    auto it = contexts_.find(ptr);

    if (it == contexts_.end())
    {
        QINMO_ERROR("Obtain context before insert.");
        it = contexts_.insert({ ptr, Context{ } }).first;
    }

    return it->second;
}

} // namespace qinmo::net::http
