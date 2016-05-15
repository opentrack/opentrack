#include "ftnoir_protocol_ft.h"
#include <windows.h>

class check_for_first_run : public runonce
{
    bool checked_for_first_run;
    bool is_first_instance;
    bool enabled;

    void try_exit()
    {
        if (is_first_instance && enabled)
        {
            qDebug() << "ft runonce: removing registry keys";
            FTNoIR_Protocol::set_protocols(false, false);
        }
    }

public:
    check_for_first_run() : checked_for_first_run(false), is_first_instance(false), enabled(false)
    {
    }

    bool is_first_run() override
    {
        return checked_for_first_run && is_first_instance;
    }

    void set_enabled(bool flag) override
    {
        enabled = flag;
        qDebug() << "ft runonce:" << "enabled" << flag;
    }

    void try_runonce() override
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

        qDebug() << "ft runonce:" << "first-run" << is_first_instance << "checked" << checked_for_first_run;
    }

    ~check_for_first_run()
    {
        try_exit();
    }
};

std::unique_ptr<runonce> FTNoIR_Protocol::runonce_check = std::unique_ptr<runonce>(static_cast<runonce*>(new check_for_first_run()));
