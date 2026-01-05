#pragma once

#include "export.hpp"
#include "compat/timer.hpp"
#include <QString>

struct OTR_INPUT_EXPORT Key
{
    QString guid;
    Timer timer;
    int keycode = 0;
    bool shift : 1 = false;
    bool ctrl  : 1 = false;
    bool alt   : 1 = false;
    bool held = true;
    bool enabled = true;

    Key();
    bool should_process();
};
