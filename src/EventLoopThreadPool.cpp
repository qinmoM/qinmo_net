#include "qinmo/net/EventLoopThreadPool.h"
#include "qinmo/tool/Logger.h"

namespace qinmo
{
/// @namespace qinmo::net
namespace net
{

EventLoopThreadPool::EventLoopThreadPool(EventLoop* mainLoop, unsigned int numSubThread)
    : started_(false)
    , next_(0)
    , baseLoop_(mainLoop)
{
    threads_.reserve(numSubThread);
    loops_.resize(numSubThread, nullptr);
}

bool EventLoopThreadPool::started() const
{
    return started_.load();
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops()
{
    return loops_.size() > 0 ? loops_ : std::vector<EventLoop*>(1, baseLoop_);
}

void EventLoopThreadPool::start(EventLoopThread::EventLoopThreadInitFunc func)
{
    if (started_.load())
    {
        QINMO_ERROR("EventLoopThreadPool:Cannot start again.");
        return;
    }

    started_.store(true);
    for (int i = 0; i < loops_.size(); ++i)
    {
        threads_.emplace_back(new EventLoopThread(func));
        loops_[i] = threads_[i]->start();
    }

    if (loops_.size() == 0)
        func(baseLoop_);
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    if (0 == loops_.size()) return baseLoop_;

    return loops_[next_++ % loops_.size()];
}

} // namespace net
} // namespace qinmo