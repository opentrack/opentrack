/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once
#include <ctime>

#if defined (_WIN32)
#   include <windows.h>
#	ifndef CLOCK_MONOTONIC
#   	define CLOCK_MONOTONIC -1
#	endif
static inline void opentrack_clock_gettime(int, struct timespec* ts)
{
    static LARGE_INTEGER freq;

    if (!freq.QuadPart)
        (void) QueryPerformanceFrequency(&freq);

    LARGE_INTEGER d;

    (void) QueryPerformanceCounter(&d);

    using ll = long long;
    using ld = long double;
    const long long part = ll(d.QuadPart / ld(freq.QuadPart) * 1000000000.L);
    using t_s = decltype(ts->tv_sec);
    using t_ns = decltype(ts->tv_nsec);

    ts->tv_sec = t_s(part / 1000000000LL);
    ts->tv_nsec = t_ns(part % 1000000000LL);
}
#	define clock_gettime opentrack_clock_gettime
#else
#   if defined(__MACH__)
#       define CLOCK_MONOTONIC 0
#       include <inttypes.h>
#       include <mach/mach_time.h>
static inline void clock_gettime(int, struct timespec* ts)
{
    static mach_timebase_info_data_t    sTimebaseInfo;
    uint64_t state, nsec;
    if ( sTimebaseInfo.denom == 0 ) {
        (void) mach_timebase_info(&sTimebaseInfo);
    }
    state = mach_absolute_time();
    nsec = state * sTimebaseInfo.numer / sTimebaseInfo.denom;
    ts->tv_sec = nsec / 1000000000L;
    ts->tv_nsec = nsec % 1000000000L;
}
#   endif
#endif
class Timer
{
private:
    struct timespec state;
    long long conv_nsecs(const struct timespec& cur) const
    {
        return (cur.tv_sec - state.tv_sec) * 1000000000LL + (cur.tv_nsec - state.tv_nsec);
    }
    long conv_usecs(const struct timespec& cur) const
    {
        return long(cur.tv_sec - state.tv_sec) * 1000000L + long(cur.tv_nsec - state.tv_nsec) / 1000l;
    }
public:
    Timer()
    {
        start();
    }
    void start()
    {
        clock_gettime(CLOCK_MONOTONIC, &state);
    }
    long long elapsed_nsecs() const
    {
        struct timespec cur;
        clock_gettime(CLOCK_MONOTONIC, &cur);
        return conv_nsecs(cur);
    }
    long elapsed_usecs() const
    {
        struct timespec cur;
        clock_gettime(CLOCK_MONOTONIC, &cur);
        return long(conv_usecs(cur));
    }
    long elapsed_ms() const
    {
        return elapsed_usecs() / 1000L;
    }
    double elapsed_seconds() const
    {
        return double(elapsed_nsecs() * 1e-9L);
    }
};
