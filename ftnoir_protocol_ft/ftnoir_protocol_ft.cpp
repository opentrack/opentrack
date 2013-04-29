/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2013	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage																		*
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
* FTServer		FTServer is the Class, that communicates headpose-data			*
*				to games, using the FreeTrackClient.dll.	         			*
********************************************************************************/
/*
	Modifications (last one on top):
	20130209 - WVR: Some games support both interfaces and cause trouble. Added ComboBox to fix this (hide one interface 
					by clearing the appropriate Registry-setting).
	20130203 - WVR: Added Tirviews and dummy checkboxes to the Settings dialog. This is necessary for CFS3 etc.
	20130125 - WVR: Upgraded to FT2.0: now the FreeTrack protocol supports all TIR-enabled games.
	20110401 - WVR: Moved protocol to a DLL, convenient for installation etc.
	20101224 - WVR: Base class is no longer inheriting QThread. sendHeadposeToGame
					is called from run() of Tracker.cpp
	20100601 - WVR: Added Mutex-bit in run(). Thought it wasn't so important (still do...). 
	20100523 - WVR: Implemented the Freetrack-protocol just like Freetrack does. Earlier 
					FaceTrackNoIR only worked with an adapted DLL, with a putdata function.
					Now it works direcly in shared memory!
*/
#include <algorithm>
#include "ftnoir_protocol_ft.h"
#include "ftnoir_csv/csv.h"

/** constructor **/
FTNoIR_Protocol::FTNoIR_Protocol()
{
	useTIRViews	= false;
	useDummyExe	= false;
	intUsedInterface = 0;

	//
	// Load the INI-settings.
	//
	loadSettings();

	ProgramName = "";
	intGameID = 0;

	viewsStart = 0;
	viewsStop = 0;

    pMemData = NULL;
    force_dummy = false;
    force_tirviews = false;

}

/** destructor **/
FTNoIR_Protocol::~FTNoIR_Protocol()
{

	qDebug()<< "~FTNoIR_Protocol: Destructor started.";

	//
	// Stop if started
	//
	if (viewsStop != NULL) {
		qDebug()<< "~FTNoIR_Protocol: Stopping TIRViews.";
		viewsStop();
		FTIRViewsLib.unload();
	}

	//
	// Destroy the File-mapping
	//
	FTDestroyMapping();
}

void FTNoIR_Protocol::Initialize()
{
	return;
}

//
// Read the game-data from CSV
//


//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Protocol::loadSettings() {
	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "FT" );
	intUsedInterface = iniFile.value ( "UsedInterface", 0 ).toInt();
	iniFile.endGroup ();

	//
	// Use the settings-section from the deprecated fake-TIR protocol, as they are most likely to be found there.
	//
	iniFile.beginGroup ( "FTIR" );
	useTIRViews	= iniFile.value ( "useTIRViews", 0 ).toBool();
	useDummyExe	= iniFile.value ( "useDummyExe", 1 ).toBool();
	iniFile.endGroup ();
}

//
// Update Headpose in Game.
//
void FTNoIR_Protocol::sendHeadposeToGame(double *headpose, double *rawheadpose ) {
float virtPosX;
float virtPosY;
float virtPosZ;

float virtRotX;
float virtRotY;
float virtRotZ;

float headPosX;
float headPosY;
float headPosZ;

float headRotX;
float headRotY;
float headRotZ;

    //
	// Scale the Raw measurements to the client measurements.
	//
    headRotX = getRadsFromDegrees(headpose[RY]);
    headRotY = getRadsFromDegrees(headpose[RX]);
    headRotZ = getRadsFromDegrees(headpose[RZ]);
    headPosX = headpose[TX] * 10;
    headPosY = headpose[TY] * 10;
    headPosZ = headpose[TZ] * 10;

    virtRotX = getRadsFromDegrees(headpose[RY]);
    virtRotY = getRadsFromDegrees(headpose[RX]);
    virtRotZ = getRadsFromDegrees(headpose[RZ]);
    virtPosX = headpose[TX] * 10;
    virtPosY = headpose[TY] * 10;
    virtPosZ = headpose[TZ] * 10;

	//
	// Check if the pointer is OK and wait for the Mutex.
	//
	if ( (pMemData != NULL) && (WaitForSingleObject(hFTMutex, 100) == WAIT_OBJECT_0) ) {

		//
		// Copy the Raw measurements directly to the client.
		//
		pMemData->data.RawX = headPosX;
		pMemData->data.RawY = headPosY;
		pMemData->data.RawZ = headPosZ;
		pMemData->data.RawPitch = headRotX;
		pMemData->data.RawYaw = headRotY;
		pMemData->data.RawRoll = headRotZ;

		//
		//
		pMemData->data.X = virtPosX;
		pMemData->data.Y = virtPosY;
		pMemData->data.Z = virtPosZ;
		pMemData->data.Pitch = virtRotX;
		pMemData->data.Yaw = virtRotY;
		pMemData->data.Roll = virtRotZ;

		//
		// Leave some values 0 yet...
		//
		pMemData->data.X1 = pMemData->data.DataID + 10;
		pMemData->data.X2 = 0;
		pMemData->data.X3 = 0;
		pMemData->data.X4 = 0;
		pMemData->data.Y1 = 0;
		pMemData->data.Y2 = 0;
		pMemData->data.Y3 = 0;
		pMemData->data.Y4 = 0;

		//
		// Check if the handle that was sent to the Game, was changed (on x64, this will be done by the ED-API)
		// If the "Report Program Name" command arrives (which is a '1', for now), raise the event from here!
		//
		//
		// The game-ID was changed?
		//
        if (intGameID != pMemData->GameID)
        {
            QString gameID = QString::number(pMemData->GameID);
            bool tirviews = false, dummy = false;
            QString gamename;
            CSV::getGameData(gameID, tirviews, dummy, pMemData->table, gamename);
            if (tirviews)
                start_tirviews();
            if (dummy)
                start_dummy();
            pMemData->GameID2 = pMemData->GameID;
            intGameID = pMemData->GameID;
            QMutexLocker foo(&this->game_name_mutex);
            connected_game = gamename;
        }

        ReleaseMutex(hFTMutex);
	}

	pMemData->data.DataID += 1;
}

//
// Set the Path variables and load the memory-mapping.
// Simplified function: No need to check if the DLL's actually exist. The are installed by the installer.
// If they are absent, something went terribly wrong anyway...
//
// Returns 'true' if all seems OK.
//
//

void FTNoIR_Protocol::start_tirviews() {
    QString aFileName = QCoreApplication::applicationDirPath() + "/TIRViews.dll";
    if ( QFile::exists( aFileName ) && !force_tirviews ) {
        force_tirviews = true;
        FTIRViewsLib.setFileName(aFileName);
        FTIRViewsLib.load();

        viewsStart = (importTIRViewsStart) FTIRViewsLib.resolve("TIRViewsStart");
        if (viewsStart == NULL) {
            qDebug() << "FTServer::run() says: TIRViewsStart function not found in DLL!";
        }
        else {
            qDebug() << "FTServer::run() says: TIRViewsStart executed!";
            viewsStart();
        }

        //
        // Load the Stop function from TIRViews.dll. Call it when terminating the thread.
        //
        viewsStop = (importTIRViewsStop) FTIRViewsLib.resolve("TIRViewsStop");
        if (viewsStop == NULL) {
            qDebug() << "FTServer::run() says: TIRViewsStop function not found in DLL!";
        }
    }
}

void FTNoIR_Protocol::start_dummy() {
    if (!force_dummy) {
        force_dummy = true;
        QString program = QCoreApplication::applicationDirPath() + "/TrackIR.exe";
        dummyTrackIR.start(program);
    
        qDebug() << "FTServer::run() says: TrackIR.exe executed!";
    }
}

bool FTNoIR_Protocol::checkServerInstallationOK()
{   
	QSettings settings("Freetrack", "FreetrackClient");							// Registry settings (in HK_USER)
	QSettings settingsTIR("NaturalPoint", "NATURALPOINT\\NPClient Location");	// Registry settings (in HK_USER)
	QString aLocation;															// Location of Client DLL

	qDebug() << "checkServerInstallationOK says: Starting Function";

	try {

		//
		// Write the path in the registry (for FreeTrack and FreeTrack20), for the game(s).
		//
		aLocation =  QCoreApplication::applicationDirPath() + "/";

		qDebug() << "checkServerInstallationOK says: used interface = " << intUsedInterface;
		switch (intUsedInterface) {
			case 0:									// Use both interfaces
				settings.setValue( "Path" , aLocation );
				settingsTIR.setValue( "Path" , aLocation );
				break;
			case 1:									// Use FreeTrack, disable TrackIR
				settings.setValue( "Path" , aLocation );
				settingsTIR.setValue( "Path" , "" );
				break;
			case 2:									// Use TrackIR, disable FreeTrack
				settings.setValue( "Path" , "" );
				settingsTIR.setValue( "Path" , aLocation );
				break;
			default:
				// should never be reached
			break;
		}

		//
		// TIRViews must be started first, or the NPClient DLL will never be loaded.
		//
		if (useTIRViews) {
            start_tirviews();
		}

		//
		// Check if TIRViews or dummy TrackIR.exe is required for this game
		//
		if (useDummyExe) {
            start_dummy();
		}


	} catch(...) {
		settings.~QSettings();
	}
	return FTCreateMapping();
}

//
// Create a memory-mapping to the FreeTrack data.
// It contains the tracking data, a handle to the main-window and the program-name of the Game!
//
//
bool FTNoIR_Protocol::FTCreateMapping()
{
bool bFirst = false;

	qDebug() << "FTCreateMapping says: Starting Function";

	//
	// A FileMapping is used to create 'shared memory' between the FTServer and the FTClient.
	//
	// Try to create a FileMapping to the Shared Memory.
	// If one already exists: close it.
	//
    hFTMemMap = CreateFileMappingA( INVALID_HANDLE_VALUE , 00 , PAGE_READWRITE , 0 ,
//		                           sizeof( TFreeTrackData ) + sizeof( HANDLE ) + 100, 
		                           sizeof( FTMemMap ), 
								   (LPCSTR) FT_MM_DATA );

	if ( hFTMemMap != 0 ) {
		bFirst = true;
		qDebug() << "FTCreateMapping says: FileMapping Created!" << hFTMemMap;
	}

	if ( ( hFTMemMap != 0 ) && ( (long) GetLastError == ERROR_ALREADY_EXISTS ) ) {
		bFirst = false;
		qDebug() << "FTCreateMapping says: FileMapping already exists!" << hFTMemMap;
		CloseHandle( hFTMemMap );
		hFTMemMap = 0;
	}

	//
	// Create a new FileMapping, Read/Write access
    //
    hFTMemMap = OpenFileMappingA( FILE_MAP_WRITE, false , (LPCSTR) FT_MM_DATA );
	if ( ( hFTMemMap != 0 ) ) {
		qDebug() << "FTCreateMapping says: FileMapping Opened:" << hFTMemMap;
        pMemData = (FTMemMap *) MapViewOfFile(hFTMemMap, FILE_MAP_WRITE, 0, 0,
//					sizeof(TFreeTrackData) + sizeof(hFTMemMap) + 100);
					sizeof(FTMemMap));
	    hFTMutex = CreateMutexA(NULL, false, FREETRACK_MUTEX);
	}


    if (!hFTMemMap || !pMemData) {
		QMessageBox::information(0, "FaceTrackNoIR error", QString("FTServer Error! \n"));
		return false;
	}

    pMemData->data.DataID = 1;
    pMemData->data.CamWidth = 100;
    pMemData->data.CamHeight = 250;
    pMemData->GameID2 = 0;
    memset(pMemData->table, 0, 8);

    return true;
}

//
// Destory the FileMapping to the shared memory
//
void FTNoIR_Protocol::FTDestroyMapping()
{
	if ( pMemData != NULL ) {
		UnmapViewOfFile ( pMemData );
	}
	
	CloseHandle( hFTMutex );
	CloseHandle( hFTMemMap );
	hFTMemMap = 0;

}

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Protocol object.

// Export both decorated and undecorated names.
//   GetProtocol     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetProtocol@0  - Common name decoration for __stdcall functions in C language.
//#pragma comment(linker, "/export:GetProtocol=_GetProtocol@0")

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocol* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Protocol;
}
