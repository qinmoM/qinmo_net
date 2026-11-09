#include "qinmo/base/Thread.h"
#include "qinmo/tool/Logger.h"

namespace qinmo
{

Thread::Thread(ThreadFunc func)
    : func_(std::move(func))
    , started_(false)
    , joined_(false)
    , id_(qinmo::detail::g_ThreadIDTypeEmpty)
{
    if (!func_)
    {
        QINMO_FATAL("Thread.func_ must is not nullptr.");
        std::cout << "Thread.func_ must is not nullptr." << std::endl;
        std::exit(-1);
    }
}

Thread::~Thread()
{
    if (started_ && !joined_.load())
        std::terminate();
}

bool Thread::started() const
{
    return started_.load();
}

bool Thread::joined() const
{
    return joined_.load();
}

qinmo::detail::ThreadIDType Thread::getID() const
{
    return id_;
}

void Thread::start()
{
    if (started_.load())
        return;

    started_.store(true);
    thread_ = std::thread(
        [this]() -> void
        {
            qinmo::detail::ThreadIDType id = qinmo::detail::getTid();

            {
                std::unique_lock<std::mutex> lock(mutex_);
                id_ = id;
            }

            cv_.notify_all();
            func_();
        }
    );

    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]() -> bool { return qinmo::detail::g_ThreadIDTypeEmpty != id_; });
}

void Thread::join()
{
    if (!started_.load() || joined_.load()) return;

    joined_.store(true);
    if (thread_.joinable())
        thread_.join();
}

} // namespace qinmo