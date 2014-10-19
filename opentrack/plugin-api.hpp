#pragma once

#include "export.hpp"

enum Axis {
    TX = 0, TY, TZ, Yaw, Pitch, Roll
};

#ifndef OPENTRACK_CROSS_ONLY
#   include "plugin-qt-api.hpp"
#endif