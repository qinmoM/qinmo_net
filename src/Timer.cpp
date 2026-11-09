#include "qinmo/net/Timer.h"
#include "qinmo/tool/Logger.h"

namespace qinmo
{
/// @namespace qinmo::net
namespace net
{

std::atomic<uint64_t> Timer::s_currSeq = 0;

Timer::Timer(TimerFunc func, Timestamp when, double intervalSeconds)
    : func_(std::move(func))
    , time_(when)
    , interval_(intervalSeconds * Timestamp::MicToSec)
    , seq_(++s_currSeq)
{
    QINMO_TRACE("Timer create. seq=", seq_);
}

Timer::~Timer()
{
    QINMO_TRACE("Timer destroy. seq=", seq_);
}

uint64_t Timer::getSequence() const
{
    return seq_;
}

Timestamp Timer::getTimestamp() const
{
    return time_;
}

int64_t Timer::getInterval() const
{
    return interval_;
}

void Timer::run()
{
    func_();
}

void Timer::reset()
{
    if (0 == interval_)
        time_ = Timestamp(0);
    else
        time_ = time_ + interval_;
}

} // namespace net
} // namespace qinmo