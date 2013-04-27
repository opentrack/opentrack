/* Copyright: "i couldn't care less what anyone does with the 5 lines of code i wrote" - mm0zct */
#include "ftnoir_tracker_rift.h"
#include "facetracknoir/global-settings.h"
	#include "OVR.h"
#include <cstdio>
using namespace OVR;

bool Rift_Tracker::isInitialised = false;

Rift_Tracker::Rift_Tracker()
{
    pSensor.Clear();
	pHMD.Clear();
	pManager.Clear();
	bEnableRoll = true;
	bEnablePitch = true;
	bEnableYaw = true;
	bEnableX = true;
	bEnableY = true;
	bEnableZ = true;
	should_quit = false;
    for (int i = 0; i < 6; i++)
        newHeadPose[i] = 0;
}

Rift_Tracker::~Rift_Tracker()
{
    pSensor.Clear();
	pHMD.Clear();
	pManager.Clear();
}


void Rift_Tracker::StartTracker(QFrame* videoFrame)
{
	//QMessageBox::warning(0,"FaceTrackNoIR Notification", "Tracking loading settings...",QMessageBox::Ok,QMessageBox::NoButton);
    loadSettings();
    //
    // Startup the Oculus SDK device handling, use the first Rift sensor we find.
    //
	if(!isInitialised){
	    System::Init(Log::ConfigureDefaultLog(LogMask_All));
		isInitialised = true;
	}

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
        SFusion.SetYawCorrectionEnabled(true);
        SFusion.SetMagReference();
    }

	return;
}

bool Rift_Tracker::GiveHeadPoseData(double *data)
{
    if (pHMD.GetPtr() != NULL) {
        Quatf hmdOrient = SFusion.GetOrientation();
        float yaw = 0.0f;
        float pitch = 0.0f;
        float roll = 0.0f;
        hmdOrient.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch , &roll);
        newHeadPose[RY] =pitch;
        newHeadPose[RZ] = roll;
        newHeadPose[RX] = yaw;
#if 0
        if (bEnableX) {
            data[TX] = newHeadPose[TX];
        }
        if (bEnableY) {
            data[TY] = newHeadPose[TY];
        }
        if (bEnableY) {
            data[TZ] = newHeadPose[TZ];
        }
#endif
        if (bEnableYaw) {
            data[RX] = newHeadPose[RX];
        }
        if (bEnablePitch) {
            data[RY] = newHeadPose[RY];
        }
        if (bEnableRoll) {
            data[RZ] = newHeadPose[RZ];
        }
    }
	return pHMD.GetPtr() != NULL;
}


//
// Load the current Settings from the currently 'active' INI-file.
//
void Rift_Tracker::loadSettings() {

	qDebug() << "FTNoIR_Tracker::loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "FTNoIR_Tracker::loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "Rift" );
	bEnableRoll = iniFile.value ( "EnableRoll", 1 ).toBool();
	bEnablePitch = iniFile.value ( "EnablePitch", 1 ).toBool();
	bEnableYaw = iniFile.value ( "EnableYaw", 1 ).toBool();
	bEnableX = iniFile.value ( "EnableX", 1 ).toBool();
	bEnableY = iniFile.value ( "EnableY", 1 ).toBool();
	bEnableZ = iniFile.value ( "EnableZ", 1 ).toBool();
	iniFile.endGroup ();
}


////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker object.

// Export both decorated and undecorated names.
//   GetTracker     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetTracker@0  - Common name decoration for __stdcall functions in C language.
//#pragma comment(linker, "/export:GetTracker=_GetTracker@0")

extern "C" FTNOIR_TRACKER_BASE_EXPORT void* CALLING_CONVENTION GetConstructor()
{
    return (ITracker*) new Rift_Tracker;
}
