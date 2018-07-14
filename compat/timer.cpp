/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "timer.hpp"
#include <cassert>
#include <cmath>

using time_type = Timer::time_type;

Timer::Timer()
{
    start();
}

void Timer::start()
{
    gettime(&state);
}

// nanoseconds

time_type Timer::elapsed_nsecs() const
{
    timespec cur{};
    gettime(&cur);
    return conv_nsecs(cur);
}

time_type Timer::conv_nsecs(const struct timespec& cur) const
{
    return (cur.tv_sec - state.tv_sec) * 1000000000LL + (cur.tv_nsec - state.tv_nsec);
}

// microseconds

double Timer::elapsed_usecs() const
{
    timespec cur{};
    gettime(&cur);
    const long long nsecs = conv_nsecs(cur);
    return nsecs * 1e-3;
}

// milliseconds

double Timer::elapsed_ms() const
{
    return elapsed_usecs() / 1000.;
}

double Timer::elapsed_seconds() const
{
    return double(elapsed_nsecs() * 1e-9);
}

// --
// platform-specific code starts here
// --

#if defined (_WIN32)
#   include <windows.h>

static LARGE_INTEGER otr_get_clock_frequency()
{
    LARGE_INTEGER freq{};
    const BOOL ret = QueryPerformanceFrequency(&freq);
    assert(ret && "QueryPerformanceFrequency failed");
    return freq;
}

static void otr_clock_gettime(timespec* ts)
{
    static const LARGE_INTEGER freq = otr_get_clock_frequency();

    LARGE_INTEGER d;
    (void) QueryPerformanceCounter(&d);

    using ll = long long;
    const auto part = ll(std::roundl((d.QuadPart * 1000000000.L) / ll(freq.QuadPart)));
    using t_s = decltype(ts->tv_sec);
    using t_ns = decltype(ts->tv_nsec);

    ts->tv_sec = t_s(part / 1000000000);
    ts->tv_nsec = t_ns(part % 1000000000);
}

#elif defined __MACH__
#   include <inttypes.h>
#   include <mach/mach_time.h>

static mach_timebase_info_data_t otr_get_mach_frequency()
{
    mach_timebase_info_data_t timebase_info;
    (void) mach_timebase_info(&timebase_info);
    return timebase_info;
}

static void otr_clock_gettime(timespec* ts)
{
    static const mach_timebase_info_data_t timebase_info = otr_get_mach_frequency();
    uint64_t state, nsec;
    state = mach_absolute_time();
    nsec = state * timebase_info.numer / timebase_info.denom;
    ts->tv_sec = nsec / 1000000000L;
    ts->tv_nsec = nsec % 1000000000L;
}

#endif

// common

void Timer::gettime(timespec* state)
{
#if defined(_WIN32) || defined(__MACH__)
    otr_clock_gettime(state);
#elif defined CLOCK_MONOTONIC
    const int res = clock_gettime(CLOCK_MONOTONIC, state);
    (void)res;
    assert(res == 0 && "must support CLOCK_MONOTONIC");
#else
#   error "timer query method not known"
#endif
}
