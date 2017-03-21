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
#include <tuple>

class OPENTRACK_COMPAT_EXPORT Timer
{
    struct timespec state;
    long long conv_nsecs(const struct timespec& cur) const;
    static void otr_clock_gettime(struct timespec* ts);
#ifdef _WIN32
    static LARGE_INTEGER otr_get_clock_frequency();
#elif defined(__MACH__)
    static mach_timebase_info_data_t otr_get_mach_frequency();
#endif

    static void wrap_gettime(struct timespec* state);

public:
    Timer();

    void start();
    long long elapsed_nsecs() const;
    double elapsed_usecs() const;
    double elapsed_ms() const;
    double elapsed_seconds() const;
};


