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
    Affinity() {}
    ~Affinity() {}
}
#endif
