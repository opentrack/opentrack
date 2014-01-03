/* Copyright: "i couldn't care less what anyone does with the 5 lines of code i wrote" - mm0zct */
#include "ftnoir_tracker_hydra.h"
#include "facetracknoir/global-settings.h"
#include "facetracknoir/rotation.h"
#include <cstdio>
#ifdef _WIN32
#   define SIXENSE_STATIC_LIB
#   define SIXENSE_UTILS_STATIC_LIB
#endif
#include <sixense.h>
#include <sixense_math.hpp>

Hydra_Tracker::Hydra_Tracker() : should_quit(false)
{
    for (int i = 0; i < 6; i++)
        newHeadPose[i] = 0;
}

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
    newHeadPose[Yaw] = ypr[0];
    newHeadPose[Pitch] = ypr[1];
	newHeadPose[Roll] = ypr[2];

	
	newHeadPose[TX] = acd.controllers[0].pos[0]/50.0f;
	newHeadPose[TY] = acd.controllers[0].pos[1]/50.0f;
	newHeadPose[TZ] = acd.controllers[0].pos[2]/50.0f;
		
    if (s.bEnableX) {
        data[TX] = newHeadPose[TX];
    }
    if (s.bEnableY) {
        data[TY] = newHeadPose[TY];
    }
    if (s.bEnableY) {
        data[TZ] = newHeadPose[TZ];
    }
    if (s.bEnableYaw) {
        data[Yaw] = newHeadPose[Yaw] * 57.295781f;
    }
    if (s.bEnablePitch) {
        data[Pitch] = newHeadPose[Pitch] * 57.295781f;
    }
    if (s.bEnableRoll) {
        data[Roll] = newHeadPose[Roll] * 57.295781f;
    }
}

extern "C" FTNOIR_TRACKER_BASE_EXPORT ITracker* CALLING_CONVENTION GetConstructor()
{
    return new Hydra_Tracker;
}
