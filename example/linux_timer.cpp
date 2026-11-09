#include <qinmo/net.h>

int main()
{
    using namespace qinmo::net;

    EventLoop loop;

    loop.timerAfter(
        5.0,
        []() -> void
        {
            qinmo::println("5 seconds are up.");
        }
    );

    int count = 0;
    EventLoop* loopPtr = &loop;
    TimerID id;
    id = loop.timerRepeatAfter(
        2.0,
        2.0,
        [&count, &id, loopPtr]() -> void
        {
            qinmo::println("2 seconds up. count=", count);
            if (++count > 6)
                loopPtr->timerCancel(id);
        }
    );

    loop.loop();

    return 0;
}