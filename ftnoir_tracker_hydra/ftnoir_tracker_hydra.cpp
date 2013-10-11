/* Copyright: "i couldn't care less what anyone does with the 5 lines of code i wrote" - mm0zct */
#include "ftnoir_tracker_hydra.h"
#include "facetracknoir/global-settings.h"
#include "facetracknoir/rotation.h"
#include <cstdio>
#ifdef _WIN32
#   define SIXENSE_STATIC_LIB
#   define SIXENSE_UTILS_STATIC_LIB
#endif
#include <sixense.h>
#include <sixense_math.hpp>
#ifdef WIN32
#include <sixense_utils/mouse_pointer.hpp>
#endif
#include <sixense_utils/derivatives.hpp>
#include <sixense_utils/button_states.hpp>
#include <sixense_utils/event_triggers.hpp>
#include <sixense_utils/controller_manager/controller_manager.hpp>

Hydra_Tracker::Hydra_Tracker()
{
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

Hydra_Tracker::~Hydra_Tracker()
{
	
	sixenseExit();
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

void Hydra_Tracker::StartTracker(QFrame* videoFrame)
{
	//QMessageBox::warning(0,"FaceTrackNoIR Notification", "Tracking loading settings...",QMessageBox::Ok,QMessageBox::NoButton);
    loadSettings();

	// Init sixense
	//QMessageBox::warning(0,"OpenTrack Info", "sixense init",QMessageBox::Ok,QMessageBox::NoButton);
	sixenseInit();
	//QMessageBox::warning(0,"OpenTrack Info", "sixense init complete, setting controller manager",QMessageBox::Ok,QMessageBox::NoButton);
	// Init the controller manager. This makes sure the controllers are present, assigned to left and right hands, and that
	// the hemisphere calibration is complete.
	//sixenseUtils::getTheControllerManager()->setGameType( sixenseUtils::ControllerManager::ONE_PLAYER_TWO_CONTROLLER );
	//sixenseUtils::getTheControllerManager()->registerSetupCallback( controller_manager_setup_callback );
	//QMessageBox::warning(0,"OpenTrack Info", "controller manager callback registered",QMessageBox::Ok,QMessageBox::NoButton);
	return;
}


bool Hydra_Tracker::GiveHeadPoseData(double *data)
{
    
	sixenseSetActiveBase(0);
	sixenseAllControllerData acd;
	sixenseGetAllNewestData( &acd );
	//sixenseUtils::getTheControllerManager()->update( &acd );

	//sixenseControllerData cd;
	//Rotation quat = Rotation(acd.controllers[0].rot_quat[1],acd.controllers[0].rot_quat[2],acd.controllers[0].rot_quat[3],acd.controllers[0].rot_quat[0]);
	sixenseMath::Matrix4 mat = sixenseMath::Matrix4(acd.controllers[0].rot_mat);// sixenseMath::Quat(acd.controllers[0].rot_quat[1],acd.controllers[0].rot_quat[2],acd.controllers[0].rot_quat[3],acd.controllers[0].rot_quat[0]);

    double yaw =  0.0f;
    double pitch = 0.0f;
    double roll = 0.0f;
	float ypr[3];
	
	mat.getEulerAngles().fill(ypr);
    newHeadPose[Yaw] = ypr[0];
    newHeadPose[Pitch] = ypr[1];
	newHeadPose[Roll] = ypr[2];

	
	newHeadPose[TX] = acd.controllers[0].pos[0]/50.0f;
	newHeadPose[TY] = acd.controllers[0].pos[1]/50.0f;
	newHeadPose[TZ] = acd.controllers[0].pos[2]/50.0f;
		
	if (bEnableX) {
        data[TX] = newHeadPose[TX];
    }
    if (bEnableY) {
        data[TY] = newHeadPose[TY];
    }
    if (bEnableY) {
        data[TZ] = newHeadPose[TZ];
    }

    if (bEnableYaw) {
        data[Yaw] = newHeadPose[Yaw] * 57.295781f;
    }
    if (bEnablePitch) {
        data[Pitch] = newHeadPose[Pitch] * 57.295781f;
    }
    if (bEnableRoll) {
        data[Roll] = newHeadPose[Roll] * 57.295781f;
    }
    
	return true;
}


//
// Load the current Settings from the currently 'active' INI-file.
//
void Hydra_Tracker::loadSettings() {

	qDebug() << "FTNoIR_Tracker::loadSettings says: Starting ";
	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "FTNoIR_Tracker::loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "Hydra" );
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

extern "C" FTNOIR_TRACKER_BASE_EXPORT ITracker* CALLING_CONVENTION GetConstructor()
{
    return new Hydra_Tracker;
}
