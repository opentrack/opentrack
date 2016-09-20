#pragma once

namespace portable
{
#ifdef _WIN32
#   include <windows.h>

    inline void sleep(unsigned milliseconds)
    {
        Sleep(milliseconds);
    }
#else
    #include <unistd.h>

    inline void sleep(unsigned milliseconds)
    {
        usleep(milliseconds * 1000U); // takes microseconds
    }
#endif
}
