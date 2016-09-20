#pragma once

#ifdef _WIN32
#   include <windows.h>
namespace portable
{
    inline void sleep(unsigned milliseconds)
    {
        Sleep(milliseconds);
    }
}
#else
#   include <unistd.h>
namespace portable {

    inline void sleep(unsigned milliseconds)
    {
        usleep(milliseconds * 1000U); // takes microseconds
    }
}
#endif
