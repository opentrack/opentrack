/* Copyright (c) 2013 mm0zct
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "ftnoir_tracker_rift_042.h"
#include "api/plugin-api.hpp"

#include <QString>

#include <OVR_CAPI.h>
#include <Kernel/OVR_Math.h>
#include <cstdio>
#include <cstring>
#include <cmath>

using namespace OVR;

rift_tracker_042::rift_tracker_042() : old_yaw(0), hmd(nullptr)
{
}

rift_tracker_042::~rift_tracker_042()
{
    ovrHmd_Destroy(hmd);
    ovr_Shutdown();
}

module_status rift_tracker_042::start_tracker(QFrame*)
{
    ovr_Initialize();
    hmd = ovrHmd_Create(0);
    if (hmd)
    {
        ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, ovrTrackingCap_Orientation);
        return status_ok();
    }
    else
        return error(tr("Unable to start Rift tracker: %1").arg(ovrHmd_GetLastError(nullptr)));
}


void rift_tracker_042::data(double *data)
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
            constexpr float c_mult = 16;
            constexpr float c_div = 1/c_mult;

            Vector3f axis;
            float angle;

            const Posef pose(ss.HeadPose.ThePose);
            pose.Rotation.GetAxisAngle(&axis, &angle);
            angle *= c_div;

            float yaw, pitch, roll;
            Quatf(axis, angle).GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch, &roll);

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
            constexpr double d2r = 180 / M_PI;
            data[Yaw] = yaw_ * -d2r;
            data[Pitch] = double(pitch) * d2r;
            data[Roll] = double(roll) * d2r;
            data[TX] = double(pose.Translation.x) * -1e2;
            data[TY] = double(pose.Translation.y) *  1e2;
            data[TZ] = double(pose.Translation.z) *  1e2;
        }
    }
}

OPENTRACK_DECLARE_TRACKER(rift_tracker_042, dialog_rift_042, rift_042Dll)
