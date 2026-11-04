#include "qinmo/net/EventLoop.h"
#include "qinmo/net/Poller.h"
#include "qinmo/tool/Logger.h"

namespace qinmo
{
/// @namespace qinmo::net
namespace net
{

EventLoop::EventLoop()
    : poller_(Poller::create(this))
    , pollerReturnTime_(0)
    , timeoutMs_(2000)
    , wakeupfd_(qinmo::detail::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC))
    , wakeupChannel_(new Channel(this, wakeupfd_))
    , timerManager_(this)
    , tid_(qinmo::detail::getTid())
    , quit_(false)
    , looping_(false)
    , pending_(false)
{
    wakeupChannel_->setReadEvent( [this](Timestamp) -> void { wakeupRead(); } );
    wakeupChannel_->enableRead();
}

EventLoop::~EventLoop()
{
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    qinmo::detail::close(wakeupfd_);
}

void EventLoop::setTimeoutMs(int ms)
{
    timeoutMs_.store(ms);
}

void EventLoop::loop()
{
    looping_.store(true);
    quit_.store(false);

    while (!quit_.load())
    {
        // clear processed events
        activeChannels_.clear();
        pollerReturnTime_ = poller_->poll(timeoutMs_.load(), activeChannels_);

        for (Channel* channel : activeChannels_)
            channel->handle(pollerReturnTime_);

        pendingFunctors();
    }

    looping_.store(false);
}

void EventLoop::quit()
{
    quit_.store(true);

    if (!isInCurrentThread())
        wakeup();
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    qinmo::detail::write(wakeupfd_, &one, sizeof(one));
    // logger
}

void EventLoop::wakeupRead()
{
    uint64_t one = 1;
    qinmo::detail::read(wakeupfd_, &one, sizeof(one));
    // logger
}

bool EventLoop::hasChannel(Channel& channel) const
{
    return poller_->findChannel(channel);
}

void EventLoop::setChannel(Channel& channel)
{
    poller_->setChannel(channel);
}

void EventLoop::removeChannel(Channel& channel)
{
    poller_->removeChannel(channel);
}

void EventLoop::runInLoop(Functor func)
{
    if (isInCurrentThread())
        func();
    else
        queueInLoop(std::move(func));
}

void EventLoop::queueInLoop(Functor func)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(std::move(func));
    }

    if (!isInCurrentThread() || pending_)
        wakeup();
}

TimerID EventLoop::timerAt(Timestamp timestamp, TimerFunc func)
{
    return timerManager_.addEvent(std::move(func), timestamp, 0.0);
}

TimerID EventLoop::timerAfter(double seconds, TimerFunc func)
{
    return timerManager_.addEvent(std::move(func), Timestamp::now() + static_cast<int64_t>(seconds * Timestamp::MicToSec), 0.0);
}

TimerID EventLoop::timerRepeatAt(Timestamp timestamp, double intervalSeconds, TimerFunc func)
{
    if (intervalSeconds <= 0.0)
    {
        QINMO_ERROR("EventLoop.timerRepeat..:The interval cannot be negative. interval:", intervalSeconds);
        intervalSeconds = 0.0;
    }

    return timerManager_.addEvent(std::move(func), timestamp, intervalSeconds);
}

TimerID EventLoop::timerRepeatAfter(double beginSeconds, double intervalSeconds, TimerFunc func)
{
    return timerRepeatAt(Timestamp::now() + static_cast<int64_t>(beginSeconds * Timestamp::MicToSec), intervalSeconds, std::move(func));
}

void EventLoop::timerCancel(TimerID id)
{
    return timerManager_.cancelEvent(id);
}

bool EventLoop::isInCurrentThread() const
{
    return tid_ == qinmo::detail::getTid();
}

void EventLoop::pendingFunctors()
{
    std::vector<Functor> functors;
    pending_.store(true);

    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const Functor& func : functors)
        func();

    pending_.store(false);
}

} // namespace net
} // namespace qinmo