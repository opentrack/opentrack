/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef PT_TIMER_H
#define PT_TIMER_H

#ifdef WIN32   // Windows system specific
#include <windows.h>
#else          // Unix based system specific
#include <sys/time.h>
#endif

// ----------------------------------------------------------------------------
// high resolution timer based on http://www.songho.ca/misc/timer/timer.html
class Timer
{
public:
    Timer();

    void start();
    void stop();
	void restart() { start(); } // for Qt compatibility
    double elapsed(); // get elapsed time in ms

protected:
    double startTime; // starting time in ms
    double endTime; // ending time in ms
    bool running;

#ifdef WIN32
    LARGE_INTEGER frequency;  // ticks per second
    LARGE_INTEGER startCount;
    LARGE_INTEGER endCount;
#else
    timeval startCount;
    timeval endCount;
#endif
};

#endif //PT_TIMER_H