/* Copyright: "i couldn't care less what anyone does with the 5 lines of code i wrote" - mm0zct */
#include "ftnoir_tracker_rift.h"
#include "facetracknoir/plugin-support.h"
#include "OVR_CAPI.h"
#include "Kernel/OVR_Math.h"
#include <cstdio>

using namespace OVR;

Rift_Tracker::Rift_Tracker() :
    should_quit(false), old_yaw(0), hmd(nullptr)
{
}

Rift_Tracker::~Rift_Tracker()
{
    ovrHmd_Destroy(hmd);
    ovr_Shutdown();
}

void Rift_Tracker::StartTracker(QFrame*)
{
    ovr_Initialize();
    hmd = ovrHmd_Create(0);
    if (hmd)
    {
        ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, ovrTrackingCap_Orientation);
    }
    else
    {
        QMessageBox::warning(0,"FaceTrackNoIR Error", "Unable to start Rift tracker",QMessageBox::Ok,QMessageBox::NoButton);
    }
}


void Rift_Tracker::GetHeadPoseData(double *data)
{
    if (hmd)
    {
	ovrHSWDisplayState hsw;	
	if (ovrHmd_GetHSWDisplayState(hmd, &hsw), hsw.Displayed)
            ovrHmd_DismissHSWDisplay(hmd);
        ovrTrackingState ss = ovrHmd_GetTrackingState(hmd, 0);
        if(ss.StatusFlags & ovrStatus_OrientationTracked) {
            auto pose = ss.HeadPose.ThePose;
            Quatf quat = pose.Orientation;
            float yaw, pitch, roll;
            quat.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch, &roll);
            if (s.useYawSpring)
            {
                yaw = old_yaw*s.persistence + (yaw-old_yaw);
                if(yaw > s.deadzone)
                    yaw -= s.constant_drift;
                if(yaw < -s.deadzone)
                    yaw += s.constant_drift;
                old_yaw=yaw;
            }
            if (s.bEnableYaw)
                data[Yaw] = yaw * -57.295781;
            if (s.bEnablePitch)
                data[Pitch] = pitch * -57.295781;
            if (s.bEnableRoll)
                data[Roll] = roll * 57.295781;
            if (s.bEnableX)
                data[TX] = pose.Position.x * 1e2;
            if (s.bEnableY)
                data[TY] = pose.Position.y * 1e2;
            if (s.bEnableX)
                data[TZ] = pose.Position.z * 1e2;
        }
    }
}

extern "C" FTNOIR_TRACKER_BASE_EXPORT ITracker* CALLING_CONVENTION GetConstructor()
{
    return new Rift_Tracker;
}
