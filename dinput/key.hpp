#pragma once

#include "export.hpp"
#include "compat/timer.hpp"
#include <QString>

struct OTR_DINPUT_EXPORT Key
{
    QString guid;
    Timer timer;
    int keycode = 0;
    bool shift = false;
    bool ctrl = false;
    bool alt = false;
    bool held = true;
    bool enabled = true;

    Key();

    bool should_process();
};
