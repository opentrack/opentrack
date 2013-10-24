/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2012	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage:			http://facetracknoir.sourceforge.net/home/default.htm		*
*																				*
* This program is free software; you can redistribute it and/or modify it		*
* under the terms of the GNU General Public License as published by the			*
* Free Software Foundation; either version 3 of the License, or (at your		*
* option) any later version.													*
*																				*
* This program is distributed in the hope that it will be useful, but			*
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY	*
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for	*
* more details.																	*
*																				*
* You should have received a copy of the GNU General Public License along		*
* with this program; if not, see <http://www.gnu.org/licenses/>.				*
*																				*
********************************************************************************/
#include "ftnoir_tracker_sm.h"
#include <QtGui>
#include <QMessageBox>
#include "facetracknoir/global-settings.h"

FTNoIR_Tracker::FTNoIR_Tracker() : lck_shm(SM_MM_DATA, SM_MUTEX, sizeof(SMMemMap))
{
    pMemData = (SMMemMap*) lck_shm.mem;
}

FTNoIR_Tracker::~FTNoIR_Tracker()
{
	qDebug() << "~FTNoIR_Tracker says: cleaning up";

	bEnableRoll = true;
	bEnablePitch = true;
	bEnableYaw = true;
	bEnableX = true;
	bEnableY = true;
	bEnableZ = true;
}

void FTNoIR_Tracker::StartTracker(QFrame *videoframe )
{
    qDebug() << "FTNoIR_Tracker::Initialize says: Starting ";
    QMessageBox::warning(this,
                         "Tracker deprecation",
                         "Non-free SM FaceAPI is deprecated, hence this annoying message.\n"
                         "It'll be removed for 2.0-final.",
                         QMessageBox::Ok, QMessageBox::NoButton);

    loadSettings();

    if ( pMemData != NULL ) {
        pMemData->command = 0;							// Reset any and all commands
        if (videoframe != NULL) {
            pMemData->handle = (HWND) videoframe->winId();		// Handle of Videoframe widget
        }
        else {
            pMemData->handle = NULL;					// reset Handle of Videoframe widget
        }
    }

    //
    // Start FTNoIR_FaceAPI_EXE.exe. The exe contains all faceAPI-stuff and is non-Qt...
    //
    // XXX TODO isolate it into separate directory
    faceAPI = new QProcess();
    faceAPI->setWorkingDirectory(QCoreApplication::applicationDirPath() + "/faceapi");
    faceAPI->start("\"" + QCoreApplication::applicationDirPath() + "/faceapi/opentrack-faceapi-wrapper" + "\"");
    // Show the video widget
    qDebug() << "FTNoIR_Tracker::Initialize says: videoframe = " << videoframe;

    if (videoframe != NULL) {
        videoframe->show();
    }
	if ( pMemData != NULL ) {
		pMemData->command = FT_SM_START;				// Start command
	}
}

void FTNoIR_Tracker::WaitForExit()
{

	qDebug() << "FTNoIR_Tracker::StopTracker says: Starting ";
	// stops the faceapi engine
	if ( pMemData != NULL ) {
//		if (exit == true) {
            pMemData->command = FT_SM_EXIT;
		//}
		//else {
		//	pMemData->command = FT_SM_STOP;				// Issue 'stop' command
		//}
	}
}

bool FTNoIR_Tracker::GiveHeadPoseData(double *data)
{
	//
	// Check if the pointer is OK and wait for the Mutex.
	//
    lck_shm.lock();

    //
    // Copy the measurements to FaceTrackNoIR.
    //
    if (bEnableX) {
        data[TX]     = pMemData->data.new_pose.head_pos.x * 100.0f;						// From meters to centimeters
    }
    if (bEnableY) {
        data[TY]     = pMemData->data.new_pose.head_pos.y * 100.0f;
    }
    if (bEnableZ) {
        data[TZ]     = pMemData->data.new_pose.head_pos.z * 100.0f;
    }
    if (bEnableYaw) {
        data[Yaw]   = pMemData->data.new_pose.head_rot.y_rads * 57.295781f;			// From rads to degrees
    }
    if (bEnablePitch) {
        data[Pitch] = pMemData->data.new_pose.head_rot.x_rads * 57.295781f;
    }
    if (bEnableRoll) {
        data[Roll]  = pMemData->data.new_pose.head_rot.z_rads * 57.295781f;
    }

    //
    // Reset the handshake, to let faceAPI know we're still here!
    //
    pMemData->handshake = 0;
    lck_shm.unlock();

    return ( pMemData->data.new_pose.confidence > 0 );
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Tracker::loadSettings() {

	qDebug() << "FTNoIR_Tracker::loadSettings says: Starting ";
	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "FTNoIR_Tracker::loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "SMTracker" );
	if (pMemData) {
		pMemData->initial_filter_level = iniFile.value ( "FilterLevel", 1 ).toInt();
	}

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
	return new FTNoIR_Tracker;
}
