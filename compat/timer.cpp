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

struct timespec Timer::get_delta() const
{
    struct timespec ts; // NOLINT
    gettime(&ts);
    return { ts.tv_sec - state.tv_sec, ts.tv_nsec - state.tv_nsec };
}

// milliseconds

double Timer::elapsed_ms() const
{
    struct timespec delta = get_delta();
    return delta.tv_sec * 1000 + delta.tv_nsec * 1e-6;
}

double Timer::elapsed_seconds() const
{
    struct timespec delta = get_delta();
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
    static const auto freq = otr_get_clock_frequency();
    LARGE_INTEGER d;
    BOOL ret = QueryPerformanceCounter(&d);
    assert(ret && "QueryPerformanceCounter failed");

    constexpr int nsec = 1'000'000 * 1000;
    state->tv_sec  = (time_t)(d.QuadPart/freq);
    state->tv_nsec = (decltype(state->tv_nsec))(d.QuadPart % freq * nsec / freq);
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

