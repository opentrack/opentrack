/* Copyright: "i couldn't care less what anyone does with the 5 lines of code i wrote" - mm0zct */
#include "ftnoir_tracker_rift_025.h"
#include "api/plugin-api.hpp"
#include <OVR.h>
#include <cstdio>
#include <cmath>


using namespace OVR;

Rift_Tracker::Rift_Tracker()
{
    pManager = NULL;
    pSensor = NULL;
    pSFusion = NULL;
    old_yaw = 0;
}

Rift_Tracker::~Rift_Tracker()
{
    if (pSensor)
        pSensor->Release();
    if (pSFusion)
        delete pSFusion;
    if (pManager)
        pManager->Release();
    System::Destroy();
}

void Rift_Tracker::start_tracker(QFrame*)
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
                QMessageBox::warning(0,"Error", "Unable to create Rift sensor",QMessageBox::Ok,QMessageBox::NoButton);
            }

        }
        else
        {
            QMessageBox::warning(0,"Error", "Unable to enumerate Rift tracker",QMessageBox::Ok,QMessageBox::NoButton);
        }
    }
    else
    {
        QMessageBox::warning(0,"Error", "Unable to start Rift tracker",QMessageBox::Ok,QMessageBox::NoButton);
    }
}


void Rift_Tracker::data(double *data)
{
    if (pSFusion != NULL && pSensor != NULL)
    {
        Quatf hmdOrient = pSFusion->GetOrientation();

        float yaw = 0, pitch = 0, roll = 0;

        hmdOrient.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch , &roll);

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

        static constexpr double r2d = 180 / M_PI;

        data[Yaw] = yaw_ * r2d;
        data[Pitch] = double(pitch) * r2d;
        data[Roll] = double(roll) * r2d;
    }
}

OPENTRACK_DECLARE_TRACKER(Rift_Tracker, TrackerControls, FTNoIR_TrackerDll)
