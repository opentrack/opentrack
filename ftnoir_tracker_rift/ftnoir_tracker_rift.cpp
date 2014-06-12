/* Copyright: "i couldn't care less what anyone does with the 5 lines of code i wrote" - mm0zct */
#include "ftnoir_tracker_rift.h"
#include "facetracknoir/global-settings.h"
#include "OVR.h"
#include <cstdio>

using namespace OVR;

Rift_Tracker::Rift_Tracker()
{
    should_quit = false;
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

void Rift_Tracker::StartTracker(QFrame*)
{
    System::Init(Log::ConfigureDefaultLog(LogMask_All));
    pManager = DeviceManager::Create();
    if (pManager != NULL)
    {
        DeviceEnumerator<OVR::SensorDevice> enumerator = pManager->EnumerateDevices<OVR::SensorDevice>();
        if (enumerator.IsAvailable())
        {
            pSensor = enumerator.CreateDevice();
          
            if (pSensor){
		pSFusion = new OVR::SensorFusion();
                pSFusion->Reset();
                pSFusion->AttachToSensor(pSensor);
            }else{
                QMessageBox::warning(0,"FaceTrackNoIR Error", "Unable to create Rift sensor",QMessageBox::Ok,QMessageBox::NoButton);
            }

        }else{
            QMessageBox::warning(0,"FaceTrackNoIR Error", "Unable to enumerate Rift tracker",QMessageBox::Ok,QMessageBox::NoButton);
        }
    }else{
		QMessageBox::warning(0,"FaceTrackNoIR Error", "Unable to start Rift tracker",QMessageBox::Ok,QMessageBox::NoButton);
	}
}


void Rift_Tracker::GetHeadPoseData(double *data)
{
    if (pSFusion != NULL && pSensor != NULL) {
        Quatf hmdOrient = pSFusion->GetOrientation();
        double newHeadPose[6];
        
        float yaw = 0.0f;
        float pitch = 0.0f;
        float roll = 0.0f;
        hmdOrient.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch , &roll);
        newHeadPose[Pitch] = pitch;
        newHeadPose[Roll] = roll;
        newHeadPose[Yaw] = yaw;
        if (s.useYawSpring)
        {
            newHeadPose[Yaw] = old_yaw*s.persistence + (yaw-old_yaw);
            if(newHeadPose[Yaw]>s.deadzone)newHeadPose[Yaw]-= s.constant_drift;
            if(newHeadPose[Yaw]<-s.deadzone)newHeadPose[Yaw]+= s.constant_drift;
            old_yaw=yaw;
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
}

extern "C" FTNOIR_TRACKER_BASE_EXPORT ITracker* CALLING_CONVENTION GetConstructor()
{
    return new Rift_Tracker;
}
