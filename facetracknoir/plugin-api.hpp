#pragma once

#include "facetracknoir/export.hpp"

#if defined(_WIN32)
#   define CALLING_CONVENTION __stdcall
#else
#   define CALLING_CONVENTION
#endif

enum Axis {
    TX = 0, TY, TZ, Yaw, Pitch, Roll
};

#ifndef OPENTRACK_CROSS_ONLY
#   include "facetracknoir/plugin-qt-api.hpp"
#endif