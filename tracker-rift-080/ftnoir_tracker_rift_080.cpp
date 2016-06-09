/* Copyright: "i couldn't care less what anyone does with the 5 lines of code i wrote" - mm0zct */
#include "ftnoir_tracker_rift_080.h"
#include "opentrack/plugin-api.hpp"
#include "OVR_CAPI.h"
#include "Extras/OVR_Math.h"
#include "OVR_CAPI_0_8_0.h"
#include <cstdio>

#ifndef M_PI
#   define M_PI 3.14159265358979323846
#endif

using namespace OVR;

Rift_Tracker::Rift_Tracker() : old_yaw(0), hmd(nullptr)
{
}

Rift_Tracker::~Rift_Tracker()
{
    if (hmd)
        ovr_Destroy(hmd);
    ovr_Shutdown();
}

void Rift_Tracker::start_tracker(QFrame*)
{
    if (!OVR_SUCCESS(ovr_Initialize(nullptr)))
        goto error;
    {
        ovrGraphicsLuid luid = {0};
        ovrResult res = ovr_Create(&hmd, &luid);
        if (OVR_SUCCESS(res))
        {
            ovr_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, ovrTrackingCap_Orientation);
        }
        else
            goto error;
    }
    return;
error:
    QMessageBox::warning(0,"Error", "Unable to start Rift tracker",QMessageBox::Ok,QMessageBox::NoButton);
}

void Rift_Tracker::data(double *data)
{
    if (hmd)
    {
        ovrTrackingState ss = ovr_GetTrackingState(hmd, 0, false);
        if (ss.StatusFlags & ovrStatus_OrientationTracked)
        {
            auto pose = ss.HeadPose.ThePose;
            Quatf quat = pose.Orientation;
            float yaw, pitch, roll;
            quat.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch, &roll);
            double yaw_ = yaw;
            if (s.useYawSpring)
            {
                yaw_ = old_yaw*s.persistence + (yaw_-old_yaw);
                if(yaw_ > s.deadzone)
                    yaw_ -= s.constant_drift;
                if(yaw_ < -s.deadzone)
                    yaw_ += s.constant_drift;
                old_yaw = yaw_;
            }
            static constexpr double d2r = 180 / M_PI;
            data[Yaw] = yaw_                   * -d2r;
            data[Pitch] = double(pitch)        *  d2r;
            data[Roll] = double(roll)          *  d2r;
            data[TX] = double(pose.Position.x) * -1e2;
            data[TY] = double(pose.Position.y) *  1e2;
            data[TZ] = double(pose.Position.z) *  1e2;
        }
    }
}

OPENTRACK_DECLARE_TRACKER(Rift_Tracker, TrackerControls, FTNoIR_TrackerDll)
