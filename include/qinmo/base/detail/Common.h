#pragma once

#include <string.h>
#include <ctime>
#include <chrono>

#if defined(__linux__)

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>

#elif defined(_WIN32)

#include <windows.h>

#endif



/// @namespace qinmo
namespace qinmo
{
/// @namespace qinmo::detail
/// @warning For internal use only, do NOT use it from outside the library
namespace detail
{

#if defined(__linux__)

inline char* strerror(int code) { return ::strerror(code); };

inline int close(int fd) { return ::close(fd); }
inline ssize_t read(int fd, void* ptr, size_t count) { return ::read(fd, ptr, count); }
inline ssize_t write(int fd, const void* ptr, size_t count) { return ::write(fd, ptr, count); }

/*
                time
*/
inline bool localtime(const time_t& time, std::tm& tm) { return nullptr != ::localtime_r(&time, &tm); }
inline bool gmtime(const time_t& time, std::tm& tm) { return nullptr != ::gmtime_r(&time, &tm); }

/*
                eventfd
*/
inline int eventfd(unsigned int initval, int flags) { return ::eventfd(initval, flags); }

/*
                thread ID in system call
*/
inline long tid() { return ::syscall(SYS_gettid); }

/*
                timerfd
*/
/// @param clockid CLOCK_MONOTONIC : only increases from 0  |  CLOCK_REALTIME : system time, affected when modify time
/// @param flags TFD_NONBLOCK : nonblock  |  TFD_CLOEXEC : auto close when sub process
inline int timerfd_create(int clockid, int flags) { return ::timerfd_create(clockid, flags); }
/// @param flags 0 : relative time  |  TFD_TIMER_ABSTIME : absolute time
inline int timerfd_settime(int fd, int flags, const itimerspec* newVal, itimerspec* oldVal) { return ::timerfd_settime(fd, flags, newVal, oldVal); }

#elif defined(_WIN32)

/*
                time
*/
inline bool localtime(const time_t& time, std::tm& tm) { return 0 == ::localtime_s(&tm, &time); }
inline bool gmtime(const time_t& time, std::tm& tm) { return 0 == ::gmtime_s(&tm, &time); }

/*
                thread ID in system call
*/
inline DWORD tid() { return ::GetCurrentThreadId(); }

#endif

} // namespace detail
} // namespace qinmo