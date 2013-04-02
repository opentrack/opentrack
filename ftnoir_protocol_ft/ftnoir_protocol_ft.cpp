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
#include "ftnoir_protocol_ft.h"
#include "csv.h"

/** constructor **/
FTNoIR_Protocol::FTNoIR_Protocol()
{
	comhandle = 0;
	useTIRViews	= false;
	useDummyExe	= false;
	intUsedInterface = 0;

	//
	// Load the INI-settings.
	//
	loadSettings();

	ProgramName = "";
	intGameID = 0;

	dummyTrackIR = 0;
	viewsStart = 0;
	viewsStop = 0;

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
	// Kill the dummy TrackIR process.
	//
	qDebug() << "~FTNoIR_Protocol() about to kill TrackIR.exe process";
	try {
		if (dummyTrackIR) {
			qDebug() << "FTServer::~FTServer() about to kill TrackIR.exe process";
//			dummyTrackIR->close();
			dummyTrackIR->kill();
		}
	} 
	catch (...)
    {
		qDebug() << "~FTServer says: some error occurred";
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
bool FTNoIR_Protocol::getGameData( QString gameID ){
QStringList gameLine;

	qDebug() << "getGameData, ID = " << gameID;

	//
	// Open the supported games list, to get the Name.
	//
	QFile file(QCoreApplication::applicationDirPath() + "/Settings/FaceTrackNoIR Supported Games.csv");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
		QString strError( "Cannot load file: FaceTrackNoIR Supported Games.csv" );
		sprintf_s(pMemData->ProgramName, 99, strError.toAscii());
		sprintf_s(pMemData->FTNVERSION, 9, "V160");

		//
		// Return true anyway, because maybe it's V160 compatible.
		//
		return true;
	}
	CSV csv(&file);
	gameLine = csv.parseLine();
	
	while (gameLine.count() > 2) {
		//qDebug() << "Column 0: " << gameLine.at(0);		// No.
		//qDebug() << "Column 1: " << gameLine.at(1);		// Game Name
		//qDebug() << "Column 2: " << gameLine.at(2);		// Game Protocol
		//qDebug() << "Column 3: " << gameLine.at(3);		// Supported since version
		//qDebug() << "Column 4: " << gameLine.at(4);		// Verified
		//qDebug() << "Column 5: " << gameLine.at(5);		// By
		//qDebug() << "Column 6: " << gameLine.at(6);		// International ID
		//qDebug() << "Column 7: " << gameLine.at(7);		// FaceTrackNoIR ID
		
		//
		// If the gameID was found, fill the shared memory
		//
		if (gameLine.count() > 6) {
			if (gameLine.at(6).compare( gameID, Qt::CaseInsensitive ) == 0) {
				if (pMemData != NULL) {		
					sprintf_s(pMemData->ProgramName, 99, gameLine.at(1).toAscii());
					sprintf_s(pMemData->FTNVERSION, 9, gameLine.at(3).toAscii());
					sprintf_s(pMemData->FTNID, 24, gameLine.at(7).toAscii());
				}
				file.close();
				return true;
			}
		}

		gameLine = csv.parseLine();
	}

	//
	// If the gameID was NOT found, fill only the name "Unknown game connected"
	//
	QString strUnknown("Unknown game connected (ID = " + gameID + ")");
	sprintf_s(pMemData->ProgramName, 99, strUnknown.toAscii());
	file.close();
	return true;
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Protocol::loadSettings() {
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

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
void FTNoIR_Protocol::sendHeadposeToGame( THeadPoseData *headpose, THeadPoseData *rawheadpose ) {
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

PDWORD_PTR MsgResult = 0;


	//
	// Scale the Raw measurements to the client measurements.
	//
	headRotX = getRadsFromDegrees(headpose->pitch);
	headRotY = getRadsFromDegrees(headpose->yaw);
	headRotZ = getRadsFromDegrees(headpose->roll);
	headPosX = headpose->x * 10;
	headPosY = headpose->y * 10;
	headPosZ = headpose->z * 10;

	virtRotX = getRadsFromDegrees(headpose->pitch);
	virtRotY = getRadsFromDegrees(headpose->yaw);
	virtRotZ = getRadsFromDegrees(headpose->roll);
	virtPosX = headpose->x * 10;
	virtPosY = headpose->y * 10;
	virtPosZ = headpose->z * 10;

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
		if (hMainWindow != pMemData->handle) {			// Handle in memory-mapping was changed!
			comhandle = (__int32) pMemData->handle;		// Get the command from the Game.
			if (comhandle == 1) {						// "Report Program Name"
				SendMessageTimeout( (HWND) hMainWindow, RegisterWindowMessageA(FT_PROGRAMID), 0, 0, 0, 2000, MsgResult);
				pMemData->handle = 0;					// Reset the command, to enable future commands...
			}
		}

		//
		// The game-ID was changed?
		//
		QString gameID = QString(pMemData->GameID);
	//QString gameID = QString("9999");

//		qDebug() << "sendHeadposeToGame: gameID = " << gameID;
		if (gameID.length() > 0) {
			if ( gameID.toInt() != intGameID ) {
				if (getGameData( gameID ) ) {
					SendMessageTimeout( (HWND) hMainWindow, RegisterWindowMessageA(FT_PROGRAMID), 0, 0, 0, 2000, MsgResult);
				}
				intGameID = gameID.toInt();
			}
		}
		else {
			intGameID = 0;
			pMemData->ProgramName[0] = NULL;
			pMemData->FTNID[0] = NULL;
			pMemData->FTNVERSION[0] = NULL;
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
bool FTNoIR_Protocol::checkServerInstallationOK( HANDLE handle )
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

			QString aFileName = QCoreApplication::applicationDirPath() + "/TIRViews.dll";
			if ( QFile::exists( aFileName ) ) {

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

		//
		// Check if TIRViews or dummy TrackIR.exe is required for this game
		//
		if (useDummyExe) {
			QString program = QCoreApplication::applicationDirPath() + "/TrackIR.exe";
			dummyTrackIR = new QProcess();
			dummyTrackIR->start(program);

			qDebug() << "FTServer::run() says: TrackIR.exe executed!";
		}


	} catch(...) {
		settings.~QSettings();
	}
	return FTCreateMapping( handle );
}

//
// Create a memory-mapping to the FreeTrack data.
// It contains the tracking data, a handle to the main-window and the program-name of the Game!
//
//
bool FTNoIR_Protocol::FTCreateMapping( HANDLE handle )
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
	hFTMemMap = OpenFileMappingA( FILE_MAP_ALL_ACCESS , false , (LPCSTR) FT_MM_DATA );
	if ( ( hFTMemMap != 0 ) ) {
		qDebug() << "FTCreateMapping says: FileMapping Opened:" << hFTMemMap;
		pMemData = (FTMemMap *) MapViewOfFile(hFTMemMap, FILE_MAP_ALL_ACCESS, 0, 0, 
//					sizeof(TFreeTrackData) + sizeof(hFTMemMap) + 100);
					sizeof(FTMemMap));
		if (pMemData != NULL) {
			if (bFirst) {
				memset(pMemData, 0, sizeof(FTMemMap));			// Write zero's, if first...
			}
			pMemData->handle = handle;	// The game uses the handle, to send a message that the Program-Name was set!
			hMainWindow = handle;
		}
	    hFTMutex = CreateMutexA(NULL, false, FREETRACK_MUTEX);
	}
	else {
		QMessageBox::information(0, "FaceTrackNoIR error", QString("FTServer Error! \n"));
		return false;
	}

	if (pMemData != NULL) {
		pMemData->data.DataID = 1;
		pMemData->data.CamWidth = 100;
		pMemData->data.CamHeight = 250;
	}

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

//
// Return a name, if present the name from the Game, that is connected...
//
void FTNoIR_Protocol::getNameFromGame( char *dest )
{   
	sprintf_s(dest, 99, "FreeTrack interface");

	qDebug() << "FTNoIR_Protocol::getNameFromGame says: Started, pMemData = " << pMemData << ", mutex = " << hFTMutex;

	//
	// Check if the pointer is OK and wait for the Mutex.
	//
//	if ( (pMemData != NULL) && (WaitForSingleObject(hFTMutex, 100) == WAIT_OBJECT_0) ) {
	if (pMemData != NULL) {
		qDebug() << "FTNoIR_Protocol::getNameFromGame says: Inside MemData";
		sprintf_s(dest, 99, "%s", pMemData->ProgramName);
	}

	return;
}


////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Protocol object.

// Export both decorated and undecorated names.
//   GetProtocol     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetProtocol@0  - Common name decoration for __stdcall functions in C language.
//#pragma comment(linker, "/export:GetProtocol=_GetProtocol@0")

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT void* CALLING_CONVENTION GetConstructor()
{
    return (IProtocol*) new FTNoIR_ProtocolDll;
}
