/* Copyright: "i couldn't care less what anyone does with the 5 lines of code i wrote" - mm0zct */
#include "ftnoir_tracker_hydra.h"
#include "api/plugin-api.hpp"
#include <cstdio>
#include <cmath>
#ifdef _WIN32
#   define SIXENSE_STATIC_LIB
#   define SIXENSE_UTILS_STATIC_LIB
#endif
#include <sixense.h>

Hydra_Tracker::Hydra_Tracker() : should_quit(false) {}

#include <sixense_math.hpp>

Hydra_Tracker::~Hydra_Tracker()
{

    sixenseExit();
}

void Hydra_Tracker::start_tracker(QFrame*)
{
    sixenseInit();
}

void Hydra_Tracker::data(double *data)
{

    sixenseSetActiveBase(0);
    sixenseAllControllerData acd;
    sixenseGetAllNewestData( &acd );
    sixenseMath::Matrix4 mat = sixenseMath::Matrix4(acd.controllers[0].rot_mat);

    float ypr[3];

    mat.getEulerAngles().fill(ypr);
    data[TX] = double(acd.controllers[0].pos[0])/50;
    data[TY] = double(acd.controllers[0].pos[1])/50;
    data[TZ] = double(acd.controllers[0].pos[2])/50;
    static constexpr double r2d = 180/M_PI;
    data[Yaw] = double(ypr[0]) * r2d;
    data[Pitch] = double(ypr[1]) * r2d;
    data[Roll] = double(ypr[2]) * r2d;
}

OPENTRACK_DECLARE_TRACKER(Hydra_Tracker, TrackerControls, FTNoIR_TrackerDll)
