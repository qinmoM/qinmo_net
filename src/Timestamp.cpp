#include "../include/qinmo/base/Timestamp.h"
#include "qinmo/base/detail/Common.h"

namespace qinmo
{

Timestamp::Timestamp() : microsecond_(0) {}

Timestamp::Timestamp(Timestamp::micro microsecondsSinceEpoch)
    : microsecond_(microsecondsSinceEpoch)
{ }

Timestamp::micro Timestamp::getMicroseconds() const
{
    return microsecond_;
}

Timestamp::micro Timestamp::getSeconds() const
{
    return microsecond_ / MicToSec;
}

std::string Timestamp::toString(bool local) const
{
    time_t t = getSeconds();
    std::tm tm;
    if (local)
        detail::localtime(t, tm);
    else
        detail::gmtime(t, tm);

    char buf[32] = "";
    int len = std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);

    return std::string(buf, len);
}

std::string Timestamp::toStringMicroseconds(bool local) const
{
    time_t t = getSeconds();
    std::tm tm;
    if (local)
        detail::localtime(t, tm);
    else
        detail::gmtime(t, tm);

    char buf[32] = "";
    int len = std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);

    if (0 == len) return "";
    len += std::snprintf(buf + len, sizeof(buf) - len, ".%06ld", getMicroseconds() % MicToSec);

    return std::string(buf, len);
}

Timestamp Timestamp::now()
{
    return Timestamp(std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
}

bool operator<(const Timestamp& a, const Timestamp& b)
{
    return a.getMicroseconds() < b.getMicroseconds();
}

bool operator==(const Timestamp& a, const Timestamp& b)
{
    return a.getMicroseconds() == b.getMicroseconds();
}

Timestamp operator+(const qinmo::Timestamp &a, int64_t b)
{
    return Timestamp(a.getMicroseconds() + b);
}

Timestamp operator-(const qinmo::Timestamp &a, int64_t b)
{
    return Timestamp(a.getMicroseconds() - b);
}

} // namespace qinmo