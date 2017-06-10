/* Copyright (c) 2017 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifdef _WIN32

#include "timer-resolution.hpp"
#include "time.hpp"
#include "compat/util.hpp"

#include <utility>

#include <QLibrary>
#include <QDebug>

#include <windows.h>

using namespace time_units;
using namespace timer_impl;

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

timer_resolution::timer_resolution()
{
    for (int k = 14; k > 0; k--)
        if (unlikely(timeEndPeriod(k) == 0))
        {
            qDebug() << "removed mm timer for" << k << "ms";
            k++;
        }

    static funptr_NtSetTimerResolution set_timer_res = get_funptr();

    // hundredth of a nanosecond
    //const ulong_ value = msecs * ulong_(10000);

    const ulong_ value = 156250; // default win32 timer length
    ntstatus_ res;
    ulong_ old_value = fail_();

    res = set_timer_res(value, true_(), &old_value);

    if (unlikely(res != 0))
    {
        old_value = fail_();
        qDebug() << "NtSetTimerResolution erred with" << res;
        return;
    }

    if (likely(std::abs(long(old_value) - long(value)) < 10000))
        return;

    // see if it stuck

    ulong_ old_value_ = fail_();

    res = set_timer_res(value, true_(), &old_value_);
    if (unlikely(res != 0))
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
                 << "current resolution" << time_cast<ms>(ns(t(old_value_) * t(100))).count() << "ms";
        old_value = fail_();
        return;
    }
}

#endif
