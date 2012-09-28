/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2010	Wim Vriend (Developing)									*
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
	20110401 - WVR: Moved protocol to a DLL, convenient for installation etc.
	20101224 - WVR: Base class is no longer inheriting QThread. sendHeadposeToGame
					is called from run() of Tracker.cpp
	20100601 - WVR: Added Mutex-bit in run(). Thought it wasn't so important (still do...). 
	20100523 - WVR: Implemented the Freetrack-protocol just like Freetrack does. Earlier 
					FaceTrackNoIR only worked with an adapted DLL, with a putdata function.
					Now it works direcly in shared memory!
*/
#include "ftnoir_protocol_ft.h"

/** constructor **/
FTNoIR_Protocol_FT::FTNoIR_Protocol_FT()
{
	comhandle = 0;
	loadSettings();
	ProgramName = "";
}

/** destructor **/
FTNoIR_Protocol_FT::~FTNoIR_Protocol_FT()
{
	//
	// Destroy the File-mapping
	//
	FTDestroyMapping();

	//
	// Free the DLL's
	//
	//////FTClientLib.unload();
}

/** helper to Auto-destruct **/
void FTNoIR_Protocol_FT::Release()
{
    delete this;
}

void FTNoIR_Protocol_FT::Initialize()
{
	return;
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Protocol_FT::loadSettings() {
}

//
// Update Headpose in Game.
//
void FTNoIR_Protocol_FT::sendHeadposeToGame( THeadPoseData *headpose, THeadPoseData *rawheadpose ) {
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

		//qDebug() << "FTServer says: pMemData.DataID =" << pMemData->data.DataID;
		//qDebug() << "FTServer says: ProgramName =" << pMemData->ProgramName;

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

		ReleaseMutex(hFTMutex);
	}

	pMemData->data.DataID += 1;
}

//
// Check if the Client DLL exists and load it (to test it), if so.
// Returns 'true' if all seems OK.
//
bool FTNoIR_Protocol_FT::checkServerInstallationOK( HANDLE handle )
{   
	QSettings settings("Freetrack", "FreetrackClient");				// Registry settings (in HK_USER)
	QString aLocation;												// Location of Client DLL
	QString aFileName;												// File Path and Name

	importProvider provider;
	char *pProvider;

	qDebug() << "FTCheckClientDLL says: Starting Function";

	try {

		//
		// Load the FreeTrackClient.dll from the current path of FaceTrackNoIR, because there is no
		// guarantee FreeTrack is also installed.
		//
		// Write this path in the registry (under FreeTrack/FreeTrackClient, for the game(s).
		//
		aLocation =  QCoreApplication::applicationDirPath() + "/";
		qDebug() << "FTCheckClientDLL says: Location of DLL =" << aLocation;

		//
		// Append a '/' to the Path and then the name of the dll.
		//
		aFileName = aLocation;
		aFileName.append(FT_CLIENT_FILENAME);
		qDebug() << "FTCheckClientDLL says: Full path of DLL =" << aFileName;
						
		if ( QFile::exists( aFileName ) ) {
			qDebug() << "FTCheckClientDLL says: DLL exists!";
			//
			// Write the path to the key in the Registry, so the game(s) can find it too...
			//
			settings.setValue( "Path" , aLocation );

			//
			// Load the DLL and map to the functions in it.
			//
			////////FTClientLib.setFileName(aFileName);
			////////FTClientLib.load();
			////////provider = (importProvider) FTClientLib.resolve("FTProvider");
			////////if (provider) {
			////////	pProvider = provider();
			////////	qDebug() << "FTCheckClientDLL says: Provider =" << pProvider;
			////////}
		}
		else {
			QMessageBox::information(0, "FaceTrackNoIR error", QString("Necessary file (FreeTrackClient.dll) was NOT found!\n"));
			return false;
		}
	} catch(...) {
		settings.~QSettings();
	}
	return FTCreateMapping( handle );
}

//
// Create a memory-mapping to the TrackIR data.
// It contains the tracking data, a handle to the main-window and the program-name of the Game!
//
//
bool FTNoIR_Protocol_FT::FTCreateMapping( HANDLE handle )
{
	qDebug() << "FTCreateMapping says: Starting Function";

	//
	// A FileMapping is used to create 'shared memory' between the FTServer and the FTClient.
	//
	// Try to create a FileMapping to the Shared Memory.
	// If one already exists: close it.
	//
	hFTMemMap = CreateFileMappingA( INVALID_HANDLE_VALUE , 00 , PAGE_READWRITE , 0 , 
		                           sizeof( TFreeTrackData ) + sizeof( HANDLE ) + 100, 
								   (LPCSTR) FT_MM_DATA );

	if ( hFTMemMap != 0 ) {
		qDebug() << "FTCreateMapping says: FileMapping Created!" << hFTMemMap;
	}

	if ( ( hFTMemMap != 0 ) && ( (long) GetLastError == ERROR_ALREADY_EXISTS ) ) {
		CloseHandle( hFTMemMap );
		hFTMemMap = 0;
	}

	//
	// Create a new FileMapping, Read/Write access
	//
	hFTMemMap = OpenFileMappingA( FILE_MAP_ALL_ACCESS , false , (LPCSTR) FT_MM_DATA );
	if ( ( hFTMemMap != 0 ) ) {
		qDebug() << "FTCreateMapping says: FileMapping Opened:" << hFTMemMap;
		pMemData = (FTMemMap *) MapViewOfFile(hFTMemMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TFreeTrackData) + sizeof(hFTMemMap) + 100);
		if (pMemData != NULL) {
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
void FTNoIR_Protocol_FT::FTDestroyMapping()
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
void FTNoIR_Protocol_FT::getNameFromGame( char *dest )
{   
	sprintf_s(dest, 99, "FreeTrack interface");

	qDebug() << "FTNoIR_Protocol_FT::getNameFromGame says: Started, pMemData = " << pMemData << ", mutex = " << hFTMutex;

	//
	// Check if the pointer is OK and wait for the Mutex.
	//
//	if ( (pMemData != NULL) && (WaitForSingleObject(hFTMutex, 100) == WAIT_OBJECT_0) ) {
	if (pMemData != NULL) {
		qDebug() << "FTNoIR_Protocol_FT::getNameFromGame says: Inside MemData";
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
#pragma comment(linker, "/export:GetProtocol=_GetProtocol@0")

FTNOIR_PROTOCOL_BASE_EXPORT IProtocolPtr __stdcall GetProtocol()
{
	return new FTNoIR_Protocol_FT;
}
