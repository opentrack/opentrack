/* Copyright (c) 2017 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "timer-resolution.hpp"

#if defined _WIN32
#   include <windows.h>

#   include <QLibrary>
#   include <QDebug>

typedef LONG (__stdcall *funptr_NtSetTimerResolution) (ULONG, BOOLEAN, PULONG);
static funptr_NtSetTimerResolution init_timer_resolution_funptr();
static funptr_NtSetTimerResolution get_funptr();

static funptr_NtSetTimerResolution init_timer_resolution_funptr()
{
    static QLibrary ntdll;
    ntdll.setLoadHints(QLibrary::PreventUnloadHint);
    ntdll.setFileName("ntdll.dll");
    const bool load = ntdll.load();
    if (!load)
    {
        qDebug() << "can't load ntdll:" << ntdll.errorString();
        return nullptr;
    }

    auto ret = reinterpret_cast<funptr_NtSetTimerResolution>(ntdll.resolve("NtSetTimerResolution"));
    if (ret == nullptr)
    {
        qDebug() << "can't find NtSetTimerResolution in ntdll:" << ntdll.errorString();
        return nullptr;
    }

    return ret;
}

static funptr_NtSetTimerResolution get_funptr()
{
    static auto ret = init_timer_resolution_funptr();
    return ret;
}

timer_resolution::timer_resolution(int msecs) : old_value(-1)
{
    if (msecs <= 0 || msecs > 30)
    {
        qDebug() << "can't set timer resolution to" << msecs << "ms";
        return;
    }

    funptr_NtSetTimerResolution f = get_funptr();
    if (f == nullptr)
        return;

    // hundredth of a nanosecond
    const ULONG value = msecs * ULONG(10000);
    NTSTATUS error = f(value, TRUE, &old_value);
    ULONG old_value_ = -1;

    if (error != 0)
    {
        old_value = -1;
        qDebug() << "NtSetTimerResolution erred with" << error;
        return;
    }

    // see if it stuck

    error = f(value, TRUE, &old_value_);
    if (error != 0)
    {
        old_value = -1;
        qDebug() << "NtSetTimerResolution check erred with" << error;
        return;
    }

    if (old_value_ != old_value)
    {
        qDebug() << "NtSetTimerResolution:"
                 << "old value didn't stick"
                 << "current resolution" << old_value_
                 << "* 100 ns";
        old_value = -1;
        return;
    }
}

timer_resolution::~timer_resolution()
{
    if (old_value != ULONG(-1))
    {
        funptr_NtSetTimerResolution f = get_funptr();
        ULONG fuzz = -1;
        (void) f(old_value, TRUE, &fuzz);
    }
}

#endif
