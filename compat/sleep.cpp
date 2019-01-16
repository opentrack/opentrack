#include "sleep.hpp"

#ifdef _WIN32
#   include <windows.h>
#else
#   include <unistd.h>
#endif

namespace portable
{
    void sleep(int milliseconds)
    {
        if (milliseconds > 0)
        {
#ifdef _WIN32

            Sleep((unsigned)milliseconds);
#else
            usleep(unsigned(milliseconds) * 1000U); // takes microseconds
#endif
        }
    }
}
