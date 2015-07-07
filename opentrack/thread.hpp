/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <QDebug>

enum {
    CORE_WORK = 1,
    CORE_IPC = 0,
};

#ifdef _WIN32
#include <windows.h>

class Affinity {
public:
    Affinity(int core = CORE_WORK)
    {
        DWORD_PTR ret = SetThreadAffinityMask(GetCurrentThread(), 1 << core);
        if (ret == 0)
            qDebug() << "SetThreadAffinityMask" << GetLastError();
        last = ret;
    }
    ~Affinity()
    {
        if (last)
            (void) SetThreadAffinityMask(GetCurrentThread(), last);
    }
private:
    DWORD_PTR last;
};

#else
class Affinity {
public:
    Affinity(int core = CORE_WORK) {}
    ~Affinity() {}
};
#endif
