/* Copyright: "i couldn't care less what anyone does with the 5 lines of code i wrote" - mm0zct */
#include "ftnoir_tracker_hydra.h"
#include "facetracknoir/plugin-support.h"
#include <cstdio>
#ifdef _WIN32
#   define SIXENSE_STATIC_LIB
#   define SIXENSE_UTILS_STATIC_LIB
#endif
#include <sixense.h>

Hydra_Tracker::Hydra_Tracker() : should_quit(false) {}

#pragma GCC diagnostic ignored "-Wreorder"
#include <sixense_math.hpp>

Hydra_Tracker::~Hydra_Tracker()
{
	
	sixenseExit();
}

void Hydra_Tracker::StartTracker(QFrame*)
{
	sixenseInit();
}

void Hydra_Tracker::GetHeadPoseData(double *data)
{
    
	sixenseSetActiveBase(0);
	sixenseAllControllerData acd;
	sixenseGetAllNewestData( &acd );
    sixenseMath::Matrix4 mat = sixenseMath::Matrix4(acd.controllers[0].rot_mat);

	float ypr[3];
	
	mat.getEulerAngles().fill(ypr);
    data[TX] = acd.controllers[0].pos[0]/50.0;
    data[TY] = acd.controllers[0].pos[1]/50.0;
    data[TZ] = acd.controllers[0].pos[2]/50.0;
    static constexpr double r2d = 57.295781;
    data[Yaw] = ypr[0] * r2d;
    data[Pitch] = ypr[1] * r2d;
    data[Roll] = ypr[2] * r2d;
}

extern "C" OPENTRACK_EXPORT ITracker* GetConstructor()
{
    return new Hydra_Tracker;
}
