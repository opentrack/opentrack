#pragma once

#ifdef _WIN32
#   include <windows.h>
#else
#   include <unistd.h>
#endif

namespace portable
{
#ifdef _WIN32
    inline void sleep(int milliseconds)
    {
        if (milliseconds > 0)
            Sleep(milliseconds);
    }
#else
    inline void sleep(int milliseconds)
    {
        if (milliseconds > 0)
            usleep(unsigned(milliseconds) * 1000U); // takes microseconds
    }
#endif
}
