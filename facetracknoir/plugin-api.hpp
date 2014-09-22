#pragma once

#include "facetracknoir/export.hpp"

enum Axis {
    TX = 0, TY, TZ, Yaw, Pitch, Roll
};

#ifndef OPENTRACK_CROSS_ONLY
#   include "facetracknoir/plugin-qt-api.hpp"
#endif