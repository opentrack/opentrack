#pragma once

#if defined _WIN32
#   include "export.hpp"

class OTR_COMPAT_EXPORT timer_resolution final
{
    unsigned long old_value;

public:
    timer_resolution(int msecs);
    ~timer_resolution();
};
#else
struct timer_resolution final
{
    inline timer_resolution(int) {}
};
#endif
