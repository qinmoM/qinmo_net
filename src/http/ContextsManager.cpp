#include "qinmo/net/http/ContextsManager.h"
#include "qinmo/net/ReactorTcpConnection.h"
#include "qinmo/tool/Logger.h"

namespace qinmo::net::http
{

thread_local std::unordered_map<ContextsManager::TcpConnect*, Context> ContextsManager::contexts_;

void ContextsManager::add(TcpPtr ptr, bool isSaveBody)
{
    if (contexts_.find(ptr.get()) != contexts_.end())
    {
        QINMO_ERROR("has been insert this context.");
        return;
    }

    QINMO_TRACE("insert a context: ", ptr->getfd());
    contexts_.insert({ ptr.get(), Context(ptr, isSaveBody) });
}

void ContextsManager::release(TcpPtr ptr)
{
    if (contexts_.find(ptr.get()) == contexts_.end())
    {
        QINMO_ERROR("context is non-exists.");
        return;
    }

    QINMO_TRACE("release context: ", ptr->getfd());
    contexts_.erase(ptr.get());
}

Context& ContextsManager::getContext(TcpPtr ptr)
{
    auto it = contexts_.find(ptr.get());

    if (it == contexts_.end())
    {
        QINMO_ERROR("Obtain context before insert.");
        it = contexts_.insert({ ptr.get(), Context(ptr) }).first;
    }

    return it->second;
}

} // namespace qinmo::net::http
