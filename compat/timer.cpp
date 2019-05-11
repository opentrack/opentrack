/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#undef NDEBUG

#include "timer.hpp"
#include <cassert>
#include <cmath>
#include <QDebug>

Timer::Timer()
{
    start();
}

void Timer::start()
{
    gettime(&state);
}

struct timespec Timer::gettime_() const
{
    struct timespec ts; // NOLINT
    gettime(&ts);
    unsigned long long a = ts.tv_sec, b = state.tv_sec;
    int c = ts.tv_nsec, d = state.tv_nsec;
    return { (time_t)(a - b), (long)(c - d) };
}

// milliseconds

double Timer::elapsed_ms() const
{
    struct timespec delta = gettime_();
    return delta.tv_sec * 1000 + delta.tv_nsec * 1e-6;
}

double Timer::elapsed_seconds() const
{
    struct timespec delta = gettime_();
    return delta.tv_sec + delta.tv_nsec * 1e-9;
}

// --
// platform-specific code starts here
// --

#if defined (_WIN32)
#   include <windows.h>

static auto otr_get_clock_frequency()
{
    LARGE_INTEGER freq{};
    BOOL ret = QueryPerformanceFrequency(&freq);
    assert(ret && "QueryPerformanceFrequency failed");
    return freq.QuadPart;
}

void Timer::gettime(timespec* state)
{
    static const unsigned long long freq = otr_get_clock_frequency();
    LARGE_INTEGER d;
    BOOL ret = QueryPerformanceCounter(&d);
    assert(ret && "QueryPerformanceCounter failed");

    constexpr int usec = 1000000;
    unsigned long long tm = d.QuadPart;
    tm *= usec;
    tm /= freq;
    tm %= usec;
    tm *= 1000;

    state->tv_sec = (time_t)((unsigned long long)d.QuadPart/freq);
    state->tv_nsec = (long)tm;
}

#elif defined __MACH__
#   include <inttypes.h>
#   include <mach/mach_time.h>

static mach_timebase_info_data_t otr_get_mach_frequency()
{
    mach_timebase_info_data_t timebase_info;
    kern_return_t status = mach_timebase_info(&timebase_info);
    assert(status == KERN_SUCCESS && "mach_timebase_info failed");
    return timebase_info;
}

void Timer::gettime(timespec* ts)
{
    static const mach_timebase_info_data_t timebase_info = otr_get_mach_frequency();
    uint64_t state, nsec;
    state = mach_absolute_time();
    nsec = state * timebase_info.numer / timebase_info.denom;
    ts->tv_sec = nsec / 1000000000UL;
    ts->tv_nsec = nsec % 1000000000UL;
}

#else

void Timer::gettime(timespec* ts)
{
    int error = clock_gettime(CLOCK_MONOTONIC, ts);
    assert(error == 0 && "clock_gettime failed");
}

#endif

// common

