/* Copyright: "i couldn't care less what anyone does with the 5 lines of code i wrote" - mm0zct */
#include "ftnoir_tracker_rift.h"
#include "facetracknoir/global-settings.h"
#include "OVR.h"
#include <cstdio>

using namespace OVR;

Rift_Tracker::Rift_Tracker()
{
    pSensor.Clear();
	pHMD.Clear();
	pManager.Clear();
	bEnableRoll = true;
	bEnablePitch = true;
	bEnableYaw = true;
#if 0
	bEnableX = true;
	bEnableY = true;
	bEnableZ = true;
#endif
	isCalibrated = false;
	should_quit = false;
    for (int i = 0; i < 6; i++)
        newHeadPose[i] = 0;
}

Rift_Tracker::~Rift_Tracker()
{
    pSensor.Clear();
	pHMD.Clear();
	pManager.Clear();
	System::Destroy();
}

/*
void controller_manager_setup_callback( sixenseUtils::ControllerManager::setup_step step ) {

		QMessageBox::warning(0,"OpenTrack Info", "controller manager callback",QMessageBox::Ok,QMessageBox::NoButton);
	if( sixenseUtils::getTheControllerManager()->isMenuVisible() ) {
		// Ask the controller manager what the next instruction string should be.
		std::string controller_manager_text_string = sixenseUtils::getTheControllerManager()->getStepString();
		QMessageBox::warning(0,"OpenTrack Info", controller_manager_text_string.c_str(),QMessageBox::Ok,QMessageBox::NoButton);
		// We could also load the supplied controllermanager textures using the filename: sixenseUtils::getTheControllerManager()->getTextureFileName();

	}
}*/

void Rift_Tracker::StartTracker(QFrame* videoFrame)
{
    //QMessageBox::warning(0,"FaceTrackNoIR Notification", "Tracking loading settings...",QMessageBox::Ok,QMessageBox::NoButton);
    loadSettings();
    //
    // Startup the Oculus SDK device handling, use the first Rift sensor we find.
    //
    System::Init(Log::ConfigureDefaultLog(LogMask_All));
    pManager = *DeviceManager::Create();
    DeviceEnumerator<HMDDevice>& enumerator = pManager->EnumerateDevices<HMDDevice>();
    if (enumerator.IsAvailable())
    {
        pHMD = *enumerator.CreateDevice();
        
        pSensor = *pHMD->GetSensor();
        
        if (pSensor){
            SFusion.AttachToSensor(pSensor);
        }else{
            QMessageBox::warning(0,"FaceTrackNoIR Error", "Unable to find Rift tracker",QMessageBox::Ok,QMessageBox::NoButton);
        }
		isCalibrated = false;
		MagCal.BeginAutoCalibration(SFusion);
       
    }
}


bool Rift_Tracker::GiveHeadPoseData(double *data)
{
    if (pHMD.GetPtr() != NULL) {

		if (MagCal.IsAutoCalibrating())
		{
			MagCal.UpdateAutoCalibration(SFusion);
			if (MagCal.IsCalibrated())
			{
				if(isCalibrated == false){
					//QMessageBox::warning(0,"OpenTrack Info", "Calibrated magnetic sensor",QMessageBox::Ok,QMessageBox::NoButton);
					//fprintf(stderr,"magnetic calibration complete\n");
			  		isCalibrated = true;
				}
				//Vector3f mc = MagCal.GetMagCenter();
				//SetAdjustMessage("Magnetometer Calibration Complete\nCenter: %f %f %f",mc.x,mc.y,mc.z);
			}
			//SetAdjustMessage("Mag has been successfully calibrated");
		}

        Quatf hmdOrient = SFusion.GetOrientation();
		
        float yaw = 0.0f;
        float pitch = 0.0f;
        float roll = 0.0f;
        hmdOrient.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch , &roll);
        newHeadPose[Yaw] = yaw;
        newHeadPose[Pitch] =pitch;
        newHeadPose[Roll] = roll;

#if 0
	
		newHeadPose[TX] = acd.controllers[0].pos[0]/50.0f;
		newHeadPose[TY] = acd.controllers[0].pos[1]/50.0f;
		newHeadPose[TZ] = acd.controllers[0].pos[2]/50.0f;
		
		//if (bEnableX) {
            data[TX] = newHeadPose[TX];
        //}
        //if (bEnableY) {
            data[TY] = newHeadPose[TY];
        //}
        //if (bEnableY) {
            data[TZ] = newHeadPose[TZ];
        //}
#endif
        if (bEnableYaw) {
            data[Yaw] = newHeadPose[Yaw] * 57.295781f;
        }
        if (bEnablePitch) {
            data[Pitch] = newHeadPose[Pitch] * 57.295781f;
        }
        if (bEnableRoll) {
            data[Roll] = newHeadPose[Roll] * 57.295781f;
        }
    }
	return pHMD.GetPtr() != NULL;
}


//
// Load the current Settings from the currently 'active' INI-file.
//
void Rift_Tracker::loadSettings() {

	qDebug() << "FTNoIR_Tracker::loadSettings says: Starting ";
	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "FTNoIR_Tracker::loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "Rift" );
	bEnableRoll = iniFile.value ( "EnableRoll", 1 ).toBool();
	bEnablePitch = iniFile.value ( "EnablePitch", 1 ).toBool();
	bEnableYaw = iniFile.value ( "EnableYaw", 1 ).toBool();
#if 0
	bEnableX = iniFile.value ( "EnableX", 1 ).toBool();
	bEnableY = iniFile.value ( "EnableY", 1 ).toBool();
	bEnableZ = iniFile.value ( "EnableZ", 1 ).toBool();
#endif
	iniFile.endGroup ();
}


////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker object.

// Export both decorated and undecorated names.
//   GetTracker     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetTracker@0  - Common name decoration for __stdcall functions in C language.
//#pragma comment(linker, "/export:GetTracker=_GetTracker@0")

extern "C" FTNOIR_TRACKER_BASE_EXPORT ITracker* CALLING_CONVENTION GetConstructor()
{
    return new Rift_Tracker;
}
