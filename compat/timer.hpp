/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "export.hpp"

#if defined (_WIN32)
#   include <windows.h>
#elif defined(__MACH__)
#   include <inttypes.h>
#   include <mach/mach_time.h>
#endif

#include <ctime>

#include "time.hpp"
#include "util.hpp"

class OTR_COMPAT_EXPORT Timer final
{
    struct timespec state;
    long long conv_nsecs(const struct timespec& cur) const;
    static void otr_clock_gettime(struct timespec* ts);
#ifdef _WIN32
    static LARGE_INTEGER otr_get_clock_frequency();
#elif defined(__MACH__)
    static mach_timebase_info_data_t otr_get_mach_frequency();
#endif

    static void gettime(struct timespec* state);

    using ns = time_units::ns;
public:
    Timer();
    void start();

    template<typename t>
    t elapsed() const
    {
        using namespace time_units;
        return time_cast<t>(ns(elapsed_nsecs()));
    }

    template<typename t>
    bool is_elapsed(const t& time_value)
    {
        using namespace time_units;

        if (unlikely(elapsed<ns>() >= time_value))
        {
            start();
            return true;
        }
        return false;
    }

    long long elapsed_nsecs() const;
    double elapsed_usecs() const;
    double elapsed_ms() const;
    double elapsed_seconds() const;
};
