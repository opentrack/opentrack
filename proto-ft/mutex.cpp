#include "ftnoir_protocol_ft.h"
#include "mutex.hpp"
#include <windows.h>
#include <QDebug>

check_for_first_run::check_for_first_run() : checked_for_first_run(false), is_first_instance(false), enabled(false)
{
}

bool check_for_first_run::is_first_run()
{
    return checked_for_first_run && is_first_instance;
}

void check_for_first_run::set_enabled(bool flag)
{
    enabled = flag;
}

void check_for_first_run::try_runonce()
{
    constexpr const char* name = "opentrack-freetrack-runonce";

    if (checked_for_first_run)
        return;

    // just leak it, no issue
    HANDLE h = CreateMutexA(nullptr, false, name);

    switch (WaitForSingleObject(h, 0))
    {
    case WAIT_OBJECT_0:
        is_first_instance = true;
        checked_for_first_run = true;
        break;
    case WAIT_TIMEOUT:
        checked_for_first_run = true;
        break;
    default:
        checked_for_first_run = false;
        break;
    }

    if (checked_for_first_run && !is_first_instance)
        CloseHandle(h);
}

check_for_first_run::~check_for_first_run()
{
    try_exit();
}

void check_for_first_run::try_exit()
{
    if (is_first_instance && enabled)
    {
        qDebug() << "ft runonce: removing registry keys";
        FTNoIR_Protocol::set_protocols(false, false);
    }
}
