#pragma once

#ifdef _WIN32
#   include <windows.h>
#else
#   include <unistd.h>
#endif

namespace portable
{
#ifdef _WIN32
    inline void sleep(unsigned milliseconds)
    {
        Sleep(milliseconds);
    }
#else
    inline void sleep(unsigned milliseconds)
    {
        usleep(milliseconds * 1000U); // takes microseconds
    }
#endif
}
