#pragma once
#include <ctime>
#if defined (_WIN32)
#   include <windows.h>
static inline void opentrack_clock_gettime(int, struct timespec* ts)
{
    static LARGE_INTEGER freq;

    if (!freq.QuadPart)
        (void) QueryPerformanceFrequency(&freq);

    LARGE_INTEGER d;

    (void) QueryPerformanceCounter(&d);

    d.QuadPart *= 1000000000L;
    d.QuadPart /= freq.QuadPart;

    ts->tv_sec = d.QuadPart / 1000000000L;
    ts->tv_nsec = d.QuadPart % 1000000000L;
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
    long conv(const struct timespec& cur)
    {
        return (cur.tv_sec - state.tv_sec) * 1000000000L + (cur.tv_nsec - state.tv_nsec);
    }
public:
    Timer() {
        start();
    }
    long start() {
        struct timespec cur;
        (void) clock_gettime(CLOCK_MONOTONIC, &cur);
        state = cur;
        int ret = conv(cur);
        return ret;
    }
    long elapsed() {
        struct timespec cur;
        (void) clock_gettime(CLOCK_MONOTONIC, &cur);
        return conv(cur);
    }
    long elapsed_ms() {
        return elapsed() / 1000000L;
    }
};
