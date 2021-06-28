#include "thread-name.hpp"
#ifdef _WIN32
#   include <QDebug>
#   include <windows.h>
#else
#   include <QThread>
#endif

namespace portable {

#ifdef _WIN32

#ifdef _MSC_VER
struct THREADNAME_INFO
{
    DWORD dwType;      // must be 0x1000
    LPCSTR szName;     // pointer to name (in user addr space)
    HANDLE dwThreadID; // thread ID (-1=caller thread)
    DWORD dwFlags;     // reserved for future use, must be zero
};

static inline
void set_curthread_name_old_(const char* name)
{
    HANDLE curthread = GetCurrentThread();
    THREADNAME_INFO info; // NOLINT(cppcoreguidelines-pro-type-member-init)
    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = curthread;
    info.dwFlags = 0;
    __try
    {
        static_assert(sizeof(info) % sizeof(unsigned) == 0);
        unsigned sz = sizeof(info)/sizeof(unsigned);
        RaiseException(0x406D1388, 0, sz, (const ULONG_PTR*)&info);
    }
    __except (EXCEPTION_CONTINUE_EXECUTION)
    {
    }
}

static inline
void set_curthread_name_old(const QString& name_)
{
    QByteArray str = name_.toLocal8Bit();
    const char* name = str.constData();

    set_curthread_name_old_(name);
}
#else

static inline void set_curthread_name_old(const QString&) {}

#endif

using SetThreadDescr_type = HRESULT (__stdcall *)(HANDLE, const wchar_t*);

static SetThreadDescr_type get_funptr()
{
    HMODULE module;
    if (GetModuleHandleExA(0, "kernel32.dll", &module))
        return (SetThreadDescr_type)GetProcAddress(module, "SetThreadDescription");
    else
        return nullptr;
}

void set_curthread_name(const QString& name)
{
    static_assert(sizeof(wchar_t) == sizeof(decltype(*QString().utf16())));
    static const SetThreadDescr_type fn = get_funptr();

    if (fn)
        fn(GetCurrentThread(), (const wchar_t*)name.utf16());
    else
        set_curthread_name_old(name);
}

#else

void set_curthread_name(const QString& name)
{
    QThread::currentThread()->setObjectName(name);
}

#endif

} // ns portable
