/* Copyright (c) 2017 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "timer-resolution.hpp"

#if defined _WIN32
#   include <QLibrary>
#   include <QDebug>

using namespace timer_impl;

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
    // starting with C++11 static initializers are fully
    // thread-safe and run the first time function is called

    // cf. http://stackoverflow.com/questions/8102125/is-local-static-variable-initialization-thread-safe-in-c11

    static auto ret = init_timer_resolution_funptr();
    return ret;
}

timer_resolution::timer_resolution(int msecs) : old_value(fail_())
{
    if (msecs <= 0 || msecs > 100)
    {
        qDebug() << "can't set timer resolution to" << msecs << "ms";
        return;
    }

    funptr_NtSetTimerResolution set_timer_res = get_funptr();
    if (set_timer_res == nullptr)
        return;

    // hundredth of a nanosecond
    const ulong_ value = msecs * ulong_(10000);
    ntstatus_ res;

    res = set_timer_res(value, true_(), &old_value);

    if (res < 0)
    {
        old_value = fail_();
        qDebug() << "NtSetTimerResolution erred with" << res;
        return;
    }

    // see if it stuck

    ulong_ old_value_ = fail_();

    res = set_timer_res(value, true_(), &old_value_);
    if (res < 0)
    {
        old_value = fail_();
        qDebug() << "NtSetTimerResolution check erred with" << res;
        return;
    }

    if (old_value_ != old_value)
    {
        using t = long long;

        qDebug() << "NtSetTimerResolution:"
                 << "old value didn't stick"
                 << "current resolution" << (t(old_value_) * t(100))
                 << "ns";
        old_value = fail_();
        return;
    }
}

timer_resolution::~timer_resolution()
{
    if (old_value != fail_())
    {
        funptr_NtSetTimerResolution f = get_funptr();
        ulong_ fuzz = fail_();
        (void) f(old_value, true_(), &fuzz);
    }
}

#endif
