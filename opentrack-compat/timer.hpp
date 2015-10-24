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

    long long part = d.QuadPart / ((long double)freq.QuadPart) * 1000000000.L;

    ts->tv_sec = part / 1000000000ULL;
    ts->tv_nsec = part % 1000000000ULL;
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
class Timer {
private:
    struct timespec state;
    long long conv(const struct timespec& cur) const
    {
        return (cur.tv_sec - state.tv_sec) * 1000000000LL + (cur.tv_nsec - state.tv_nsec);
    }
public:
    Timer() {
        start();
    }
    void start() {
        (void) clock_gettime(CLOCK_MONOTONIC, &state);
    }
    long long elapsed() const {
        struct timespec cur;
        (void) clock_gettime(CLOCK_MONOTONIC, &cur);
        return conv(cur);
    }
    long elapsed_ms() const {
        return elapsed() / 1000000L;
    }
};
