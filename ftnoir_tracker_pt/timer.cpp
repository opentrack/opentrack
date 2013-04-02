/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "timer.h"

#include <stdlib.h>

// ----------------------------------------------------------------------------
Timer::Timer() 
: startTime(0), endTime(0), running(false)
{
#ifdef WIN32
    QueryPerformanceFrequency(&frequency);
    startCount.QuadPart = 0;
    endCount.QuadPart = 0;
#else
    startCount.tv_sec = startCount.tv_usec = 0;
    endCount.tv_sec = endCount.tv_usec = 0;
#endif
}


void Timer::start()
{
#ifdef WIN32
    QueryPerformanceCounter(&startCount);
#else
    gettimeofday(&startCount, NULL);
#endif
	running = true;
}


void Timer::stop()
{
#ifdef WIN32
    QueryPerformanceCounter(&endCount);
#else
    gettimeofday(&endCount, NULL);
#endif
	running = false;
}


double Timer::elapsed()
{
#ifdef WIN32
    if (running)
        QueryPerformanceCounter(&endCount);

    startTime = startCount.QuadPart * (1e3 / frequency.QuadPart);
    endTime = endCount.QuadPart * (1e3 / frequency.QuadPart);
#else
	(void) gettimeofday(&endCount, NULL);

    startTime = (startCount.tv_sec * 1e3) + startCount.tv_usec;
    endTime = (endCount.tv_sec * 1e3) + endCount.tv_usec;
#endif

    return endTime - startTime;
}