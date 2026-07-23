#if defined(_WIN32)

#include "qinmo/net/detail/Wrapper.h"
#include "qinmo/base/Logger.h"



/// @namespace qinmo::net::detail
/// @warning For internal use only, do NOT use it from outside the library
namespace qinmo::net::detail
{

struct WindowsNetInitiator
{
    WindowsNetInitiator()
    {
        WSADATA data;
        if (0 != WSAStartup(MAKEWORD(2, 2), &data))
        {
            QINMO_FATAL("Failed to call WSAStartup in the Windows mode.");
            exit(-1);
        }
    }

    ~WindowsNetInitiator()
    {
        WSACleanup();
    }
};

WindowsNetInitiator windowsNetInitiator = WindowsNetInitiator();

} // namespace qinmo::net::detail

#endif