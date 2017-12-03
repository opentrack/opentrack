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

#include "ftnoir_tracker_rift_080.h"
#include "api/plugin-api.hpp"
#include "compat/util.hpp"

#include <QString>

#include <Extras/OVR_Math.h>
#include <OVR_CAPI_0_8_0.h>

using namespace OVR;

rift_tracker_080::rift_tracker_080() : old_yaw(0), hmd(nullptr)
{
}

rift_tracker_080::~rift_tracker_080()
{
    if (hmd)
        ovr_Destroy(hmd);
    ovr_Shutdown();
}

module_status rift_tracker_080::start_tracker(QFrame*)
{
    ovrResult code;
    ovrGraphicsLuid luid = {{0}};

    code = ovr_Initialize(nullptr);
    if (!OVR_SUCCESS(code))
        goto error;

    code = ovr_Create(&hmd, &luid);
    if (!OVR_SUCCESS(code))
        goto error;

    ovr_ConfigureTracking(hmd,
                          ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position,
                          ovrTrackingCap_Orientation);

    return status_ok();
error:
    ovrErrorInfo err;
    err.Result = code;
    err.ErrorString[0] = '\0';
    ovr_GetLastErrorInfo(&err);

    QString strerror(err.ErrorString);
    if (strerror.size() == 0)
        strerror = "Unknown reason";

    return error(strerror);
}

void rift_tracker_080::data(double *data)
{
    if (hmd)
    {
        ovrTrackingState ss = ovr_GetTrackingState(hmd, 0, false);
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
            Quatf(axis, angle).GetYawPitchRoll(&yaw, &pitch, &roll);

            yaw *= c_mult;
            pitch *= c_mult;
            roll *= c_mult;

            double yaw_ = double(yaw);
            if (s.useYawSpring)
            {
                yaw_ = old_yaw*s.persistence + (yaw_-old_yaw);
                if(yaw_ > s.deadzone)
                    yaw_ -= s.constant_drift;
                if(yaw_ < -s.deadzone)
                    yaw_ += s.constant_drift;
                old_yaw = yaw_;
            }
            constexpr double d2r = 180 / M_PI;
            data[Yaw] = yaw_                   * -d2r;
            data[Pitch] = double(pitch)        *  d2r;
            data[Roll] = double(roll)          *  d2r;
            data[TX] = double(pose.Translation.x) * -1e2;
            data[TY] = double(pose.Translation.y) *  1e2;
            data[TZ] = double(pose.Translation.z) *  1e2;
        }
    }
}

OPENTRACK_DECLARE_TRACKER(rift_tracker_080, dialog_rift_080, rift_080Dll)
