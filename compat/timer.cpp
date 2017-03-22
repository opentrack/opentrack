/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "timer.hpp"

Timer::Timer()
{
    start();
}

void Timer::start()
{
    wrap_gettime(&state);
}

// common

void Timer::wrap_gettime(timespec* state)
{
#if defined(_WIN32) || defined(__MACH__)
    otr_clock_gettime(state);
#else
    (void) clock_gettime(CLOCK_MONOTONIC, state);
#endif
}

// nanoseconds

long long Timer::elapsed_nsecs() const
{
    struct timespec cur = {};
    wrap_gettime(&cur);
    return conv_nsecs(cur);
}

long long Timer::conv_nsecs(const timespec& cur) const
{
    // this can and will overflow
    return (cur.tv_sec - state.tv_sec) * 1000000000LL + (cur.tv_nsec - state.tv_nsec);
}

// microseconds

double Timer::elapsed_usecs() const
{
    struct timespec cur = {};
    wrap_gettime(&cur);
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
    return double(elapsed_nsecs() * 1e-9L);
}

// --
// platform-specific code starts here
// --

#if defined _WIN32
LARGE_INTEGER Timer::otr_get_clock_frequency()
{
    LARGE_INTEGER freq = {};
    (void) QueryPerformanceFrequency(&freq);
    return freq;
}

void Timer::otr_clock_gettime(timespec* ts)
{
    static LARGE_INTEGER freq = otr_get_clock_frequency();

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

#elif defined __MACH__
mach_timebase_info_data_t Timer::otr_get_mach_frequency()
{
    mach_timebase_info_data_t timebase_info;
    (void) mach_timebase_info(&timebase_info);
    return timebase_info;
}

double Timer::otr_clock_gettime(timespec* ts)
{
    static mach_timebase_info_data_t timebase_info = otr_get_mach_frequency();
    uint64_t state, nsec;
    state = mach_absolute_time();
    nsec = state * timebase_info.numer / timebase_info.denom;
    ts->tv_sec = nsec / 1000000000L;
    ts->tv_nsec = nsec % 1000000000L;
}

#endif
