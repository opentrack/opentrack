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

FTNoIR_Tracker::FTNoIR_Tracker()
{
}

FTNoIR_Tracker::~FTNoIR_Tracker()
{
	qDebug() << "~FTNoIR_Tracker says: cleaning up";

	if ( pMemData != NULL ) {
		UnmapViewOfFile ( pMemData );
	}

	numTracker = 1;
	bEnableRoll = true;
	bEnablePitch = true;
	bEnableYaw = true;
	bEnableX = true;
	bEnableY = true;
	bEnableZ = true;

	dInvertRoll = 1.0f;
	dInvertPitch = 1.0f;
	dInvertYaw = 1.0f;
	dInvertX = 1.0f;
	dInvertY = 1.0f;
	dInvertZ = 1.0f;
	
	CloseHandle( hSMMutex );
	CloseHandle( hSMMemMap );
	hSMMemMap = 0;
}

void FTNoIR_Tracker::Initialize( QFrame *videoframe, int num)
{
	qDebug() << "FTNoIR_Tracker::Initialize says: Starting ";

	if (SMCreateMapping()) {
		qDebug() << "FTNoIR_Tracker::Initialize Mapping created.";
	}
	else {
		QMessageBox::warning(0,"FaceTrackNoIR Error","Memory mapping not created!",QMessageBox::Ok,QMessageBox::NoButton);
	}

	numTracker = num;
	loadSettings();

	if ( pMemData != NULL ) {
		pMemData->command = 0;							// Reset any and all commands
		if (videoframe != NULL) {
			pMemData->handle = videoframe->winId();		// Handle of Videoframe widget
		}
		else {
			pMemData->handle = NULL;					// reset Handle of Videoframe widget
		}
	}

	//
	// Start FTNoIR_FaceAPI_EXE.exe. The exe contains all faceAPI-stuff and is non-Qt...
	//
	QString program = "FTNoIR_FaceAPI_EXE.exe";
	faceAPI = new QProcess(0);
	faceAPI->start(program);

	// Show the video widget
	qDebug() << "FTNoIR_Tracker::Initialize says: videoframe = " << videoframe;

	if (videoframe != NULL) {
		videoframe->show();
	}
	return;
}

void FTNoIR_Tracker::StartTracker( HWND parent_window )
{
	if ( pMemData != NULL ) {
		pMemData->command = FT_SM_START;				// Start command
	}
	return;
}

void FTNoIR_Tracker::StopTracker( bool exit )
{

	qDebug() << "FTNoIR_Tracker::StopTracker says: Starting ";
	// stops the faceapi engine
	if ( pMemData != NULL ) {
//		if (exit == true) {
			pMemData->command = (exit) ? FT_SM_EXIT : FT_SM_STOP;			// Issue 'stop' command
		//}
		//else {
		//	pMemData->command = FT_SM_STOP;				// Issue 'stop' command
		//}
	}
	return;
}

bool FTNoIR_Tracker::GiveHeadPoseData(THeadPoseData *data)
{
	//
	// Check if the pointer is OK and wait for the Mutex.
	//
	if ( (pMemData != NULL) && (WaitForSingleObject(hSMMutex, 100) == WAIT_OBJECT_0) ) {

//		qDebug() << "FTNoIR_Tracker::GiveHeadPoseData says: Retrieving data.";
		
		//
		// Copy the measurements to FaceTrackNoIR.
		//
		if (bEnableX) {
			data->x     = dInvertX * pMemData->data.new_pose.head_pos.x * 100.0f;						// From meters to centimeters
		}
		if (bEnableY) {
			data->y     = dInvertY * pMemData->data.new_pose.head_pos.y * 100.0f;
		}
		if (bEnableZ) {
			data->z     = dInvertZ * pMemData->data.new_pose.head_pos.z * 100.0f;
		}
		if (bEnableYaw) {
			data->yaw   = dInvertYaw * pMemData->data.new_pose.head_rot.y_rads * 57.295781f;			// From rads to degrees
		}
		if (bEnablePitch) {
			data->pitch = dInvertPitch * pMemData->data.new_pose.head_rot.x_rads * 57.295781f;
		}
		if (bEnableRoll) {
			data->roll  = dInvertRoll * pMemData->data.new_pose.head_rot.z_rads * 57.295781f;
		}

		//
		// Reset the handshake, to let faceAPI know we're still here!
		//
		pMemData->handshake = 0;

		ReleaseMutex(hSMMutex);
		return ( pMemData->data.new_pose.confidence > 0 );
	}
	return false;
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Tracker::loadSettings() {
int numRoll;											// Number of Tracker (1 or 2) which tracks this axis
int numPitch;
int numYaw;
int numX;
int numY;
int numZ;

	qDebug() << "FTNoIR_Tracker::loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "FTNoIR_Tracker::loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "SMTracker" );
	if (pMemData) {
		pMemData->initial_filter_level = iniFile.value ( "FilterLevel", 1 ).toInt();
	}
	dInvertRoll = (iniFile.value ( "InvertRoll", 0 ).toBool()) ? -1.0f : 1.0f;
	dInvertPitch = (iniFile.value ( "InvertPitch", 0 ).toBool()) ? -1.0f : 1.0f;
	dInvertYaw = (iniFile.value ( "InvertYaw", 0 ).toBool()) ? -1.0f : 1.0f;
	dInvertX = (iniFile.value ( "InvertX", 0 ).toBool()) ? -1.0f : 1.0f;
	dInvertY = (iniFile.value ( "InvertY", 0 ).toBool()) ? -1.0f : 1.0f;
	dInvertZ = (iniFile.value ( "InvertZ", 0 ).toBool()) ? -1.0f : 1.0f;

	iniFile.endGroup ();

	iniFile.beginGroup ( "HeadTracker" );
	//
	// Check if the Tracker is the Primary one.
	// If the property is not found in the INI-file, set the value.
	//
	if (numTracker == 1) {
		numRoll = iniFile.value ( "RollTracker", 1 ).toInt();
		numPitch = iniFile.value ( "PitchTracker", 1 ).toInt();
		numYaw = iniFile.value ( "YawTracker", 1 ).toInt();
		numX = iniFile.value ( "XTracker", 0 ).toInt();
		numY = iniFile.value ( "YTracker", 0 ).toInt();
		numZ = iniFile.value ( "ZTracker", 0 ).toInt();
	}
	else {
		numRoll = iniFile.value ( "RollTracker", 0 ).toInt();
		numPitch = iniFile.value ( "PitchTracker", 0 ).toInt();
		numYaw = iniFile.value ( "YawTracker", 0 ).toInt();
		numX = iniFile.value ( "XTracker", 0 ).toInt();
		numY = iniFile.value ( "YTracker", 0 ).toInt();
		numZ = iniFile.value ( "ZTracker", 0 ).toInt();
	}
	bEnableRoll = (numRoll == numTracker);
	bEnablePitch = (numPitch == numTracker);
	bEnableYaw = (numYaw == numTracker);
	bEnableX = (numX == numTracker);
	bEnableY = (numY == numTracker);
	bEnableZ = (numZ == numTracker);

	iniFile.endGroup ();

}

//
// Create a memory-mapping to the faceAPI data.
// It contains the tracking data, a command-code from FaceTrackNoIR
//
//
bool FTNoIR_Tracker::SMCreateMapping()
{
	qDebug() << "FTNoIR_Tracker::FTCreateMapping says: Starting Function";

	//
	// A FileMapping is used to create 'shared memory' between the faceAPI and FaceTrackNoIR.
	//
	// Try to create a FileMapping to the Shared Memory.
	// If one already exists: close it.
	//
	hSMMemMap = CreateFileMappingA( INVALID_HANDLE_VALUE , 00 , PAGE_READWRITE , 0 , 
		                           sizeof( TFaceData ) + sizeof( HANDLE ) + 100, 
								   (LPCSTR) SM_MM_DATA );

	if ( hSMMemMap != 0 ) {
		qDebug() << "FTNoIR_Tracker::FTCreateMapping says: FileMapping Created!";
	}

	if ( ( hSMMemMap != 0 ) && ( (long) GetLastError == ERROR_ALREADY_EXISTS ) ) {
		CloseHandle( hSMMemMap );
		hSMMemMap = 0;
	}

	//
	// Create a new FileMapping, Read/Write access
	//
	hSMMemMap = OpenFileMappingA( FILE_MAP_ALL_ACCESS , false , (LPCSTR) SM_MM_DATA );
	if ( ( hSMMemMap != 0 ) ) {
		qDebug() << "FTNoIR_Tracker::FTCreateMapping says: FileMapping Created again..." << hSMMemMap;
		pMemData = (SMMemMap *) MapViewOfFile(hSMMemMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TFaceData));
		if (pMemData != NULL) {
			qDebug() << "FTNoIR_Tracker::FTCreateMapping says: MapViewOfFile OK.";
//			pMemData->handle = handle;	// The game uses the handle, to send a message that the Program-Name was set!
		}
	    hSMMutex = CreateMutexA(NULL, false, SM_MUTEX);
	}
	else {
		qDebug() << "FTNoIR_Tracker::FTCreateMapping says: Error creating Shared Memory for faceAPI!";
		return false;
	}

	//if (pMemData != NULL) {
	//	pMemData->data.DataID = 1;
	//	pMemData->data.CamWidth = 100;
	//	pMemData->data.CamHeight = 250;
	//}

	return true;
}



////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker object.

// Export both decorated and undecorated names.
//   GetTracker     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetTracker@0  - Common name decoration for __stdcall functions in C language.
#pragma comment(linker, "/export:GetTracker=_GetTracker@0")

FTNOIR_TRACKER_BASE_EXPORT ITrackerPtr __stdcall GetTracker()
{
	return new FTNoIR_Tracker;
}
