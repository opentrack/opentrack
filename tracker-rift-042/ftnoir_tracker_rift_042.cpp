/* Copyright: "i couldn't care less what anyone does with the 5 lines of code i wrote" - mm0zct */
#include "ftnoir_tracker_rift_042.h"
#include "api/plugin-api.hpp"

#include <QString>

#include <OVR_CAPI.h>
#include <Kernel/OVR_Math.h>
#include <cstdio>
#include <cstring>
#include <cmath>

using namespace OVR;

Rift_Tracker::Rift_Tracker() : old_yaw(0), hmd(nullptr)
{
}

Rift_Tracker::~Rift_Tracker()
{
    ovrHmd_Destroy(hmd);
    ovr_Shutdown();
}

void Rift_Tracker::start_tracker(QFrame*)
{
    ovr_Initialize();
    hmd = ovrHmd_Create(0);
    if (hmd)
    {
        ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, ovrTrackingCap_Orientation);
    }
    else
    {
        QMessageBox::warning(nullptr,
                             "Error",
                             QStringLiteral("Unable to start Rift tracker: %1").arg(ovrHmd_GetLastError(nullptr)),
                             QMessageBox::Ok,
                             QMessageBox::NoButton);
    }
}


void Rift_Tracker::data(double *data)
{
    if (hmd)
    {
        ovrHSWDisplayState hsw;
        std::memset(&hsw, 0, sizeof(hsw));
        ovrHmd_GetHSWDisplayState(hmd, &hsw);
        if (hsw.Displayed)
            ovrHmd_DismissHSWDisplay(hmd);
        ovrTrackingState ss = ovrHmd_GetTrackingState(hmd, 0);
        if (ss.StatusFlags & ovrStatus_OrientationTracked)
        {
            auto pose = ss.HeadPose.ThePose;
            Quatf quat = pose.Orientation;
            float yaw, pitch, roll;
            quat.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch, &roll);
            double yaw_ = double(yaw);
            if (s.useYawSpring)
            {
                yaw_ = old_yaw*s.persistence + (yaw_ - old_yaw);
                if (yaw_ > s.deadzone)
                    yaw_ -= s.constant_drift;
                if (yaw_ < -s.deadzone)
                    yaw_ += s.constant_drift;
                old_yaw = yaw_;
            }
            static constexpr double d2r = 180 / M_PI;
            data[Yaw] = yaw_ * -d2r;
            data[Pitch] = double(pitch) * d2r;
            data[Roll] = double(roll) * d2r;
            data[TX] = double(pose.Position.x) * -1e2;
            data[TY] = double(pose.Position.y) *  1e2;
            data[TZ] = double(pose.Position.z) *  1e2;
        }
    }
}

OPENTRACK_DECLARE_TRACKER(Rift_Tracker, TrackerControls, FTNoIR_TrackerDll)
