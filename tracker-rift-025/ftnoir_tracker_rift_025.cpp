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

#include "ftnoir_tracker_rift_025.h"
#include "api/plugin-api.hpp"
#include <OVR.h>
#include <cstdio>
#include <cmath>


using namespace OVR;

rift_tracker_025::rift_tracker_025()
{
    pManager = NULL;
    pSensor = NULL;
    pSFusion = NULL;
    old_yaw = 0;
}

rift_tracker_025::~rift_tracker_025()
{
    if (pSensor)
        pSensor->Release();
    if (pSFusion)
        delete pSFusion;
    if (pManager)
        pManager->Release();
    System::Destroy();
}

void rift_tracker_025::start_tracker(QFrame*)
{
    System::Init(Log::ConfigureDefaultLog(LogMask_All));
    pManager = DeviceManager::Create();
    if (pManager != NULL)
    {
        DeviceEnumerator<OVR::SensorDevice> enumerator = pManager->EnumerateDevices<OVR::SensorDevice>();
        if (enumerator.IsAvailable())
        {
            pSensor = enumerator.CreateDevice();

            if (pSensor)
            {
                pSFusion = new OVR::SensorFusion();
                pSFusion->Reset();
                pSFusion->AttachToSensor(pSensor);
            }
            else
            {
                QMessageBox::warning(nullptr,
                                     QCoreApplication::translate("rift_tracker_025", "Error"),
                                     QCoreApplication::translate("rift_tracker_025", "Unable to create Rift sensor"),
                                     QMessageBox::Ok,QMessageBox::NoButton);
            }

        }
        else
        {
            QMessageBox::warning(nullptr,
                                 QCoreApplication::translate("rift_tracker_025", "Error"),
                                 QCoreApplication::translate("rift_tracker_025", "Unable to enumerate Rift tracker"),
                                 QMessageBox::Ok,QMessageBox::NoButton);
        }
    }
    else
    {
        QMessageBox::warning(nullptr,
                             QCoreApplication::translate("rift_tracker_025", "Error"),
                             QCoreApplication::translate("rift_tracker_025", "Unable to start Rift tracker"),
                             QMessageBox::Ok,QMessageBox::NoButton);
    }
}


void rift_tracker_025::data(double *data)
{
    if (pSFusion != NULL && pSensor != NULL)
    {
        Quatf rot = pSFusion->GetOrientation();

        constexpr float c_mult = 8;
        constexpr float c_div = 1/c_mult;

        Vector3f axis;
        float angle;

        rot.GetAxisAngle(&axis, &angle);
        angle *= c_div;

        float yaw, pitch, roll;
        Quatf(axis, angle).GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch, &roll);

        double yaw_ = double(yaw);

        if (s.useYawSpring)
        {
            yaw_ = old_yaw*s.persistence + (yaw_-old_yaw);
            if (yaw_ > s.deadzone)
                yaw_ -= s.constant_drift;
            if (yaw_ < -s.deadzone)
                yaw_ += s.constant_drift;
            old_yaw = yaw_;
        }

        constexpr double r2d = 180 / M_PI;

        data[Yaw] = yaw_ * r2d;
        data[Pitch] = double(pitch) * r2d;
        data[Roll] = double(roll) * r2d;
    }
}

OPENTRACK_DECLARE_TRACKER(rift_tracker_025, dialog_rift_025, rift_025Dll)
