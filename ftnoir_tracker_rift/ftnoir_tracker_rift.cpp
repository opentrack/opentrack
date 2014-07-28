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
        ovrHmd_GetDesc(hmd, &hmdDesc);
        ovrHmd_StartSensor(hmd, ovrSensorCap_Orientation| ovrSensorCap_YawCorrection | ovrSensorCap_Position, ovrSensorCap_Orientation);
    }else{
		QMessageBox::warning(0,"FaceTrackNoIR Error", "Unable to start Rift tracker",QMessageBox::Ok,QMessageBox::NoButton);
	}
}


void Rift_Tracker::GetHeadPoseData(double *data)
{
    if (hmd)
    {
        frameTiming = ovrHmd_BeginFrameTiming(hmd, 0); 
        ovrSensorState ss = ovrHmd_GetSensorState(hmd, frameTiming.ScanoutMidpointSeconds);
        ovrHmd_EndFrameTiming(hmd);
        if(ss.StatusFlags & ovrStatus_OrientationTracked) {
            ovrPosef pose = ss.Predicted.Pose;
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
                data[Yaw] = yaw * 57.295781f;
            if (s.bEnablePitch)
                data[Pitch] = pitch * 57.295781f;
            if (s.bEnableRoll)
                data[Roll] = roll * 57.295781f;
            if (s.bEnableX)
                data[TX] = pose.Position.x * 1e-3;
            if (s.bEnableY)
                data[TY] = pose.Position.y * 1e-3;
            if (s.bEnableX)
                data[TZ] = pose.Position.z * 1e-3;
        }
    }
}

extern "C" FTNOIR_TRACKER_BASE_EXPORT ITracker* CALLING_CONVENTION GetConstructor()
{
    return new Rift_Tracker;
}
