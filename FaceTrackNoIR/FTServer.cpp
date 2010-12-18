/********************************************************************************
* FTServer			FTServer is the Class, that communicates headpose-data		*
*					to games, using the FreeTrackClient.dll.         			*
*					It was (very) loosely translated from FTServer.pas.			*
*					which was created by the FreeTrack-team.					*
*																				*
* Copyright (C) 2010	Wim Vriend (Developing)									*
*						Ron Hendriks (Testing and Research)						*
*																				*
* Homepage				<http://www.free-track.net>								*
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
* We would like to extend our grattitude to the creators of SweetSpotter,		*
* which has become the basis of this program: "Great work guys!"				*
********************************************************************************/
//
// There are some issues, that may need solving:
//
//		The FaceAPI only runs in the 'Release' configuration. It may have something to
//		do with compiler-settings, but for now it's not solved...
//			--> WVR 20101023: This was solved by downgrading to VS2005...
//
//		Connecting with the Freetrack DLL only seems to work, when UNICODE is OFF.
//		That's strange, isn't it?
//			--> WVR 20100409: Update on this: Loading the DLL with QLibrary implemented.
//							  With UNICODE on, the DLL is loaded fine, but the memory-mapping
//							  does not. Explicitely using the 'A' extension for CreateFileMapping
//							  solves this (for now.) Maybe there is a Qt substitute for this?
//
/*
	Modifications (last one on top):
		20100601 - WVR: Added Mutex-bit in run(). Thought it wasn't so important (still do...). 
		20100523 - WVR: Implemented the Freetrack-protocol just like Freetrack does. Earlier 
						FaceTrackNoIR only worked with an adapted DLL, with a putdata function.
						Now it works direcly in shared memory!
*/
#include "FTServer.h"

/** constructor **/
FTServer::FTServer() {

	// Create events
	m_StopThread = CreateEvent(0, TRUE, FALSE, 0);
	m_WaitThread = CreateEvent(0, TRUE, FALSE, 0);

	ProgramName = "";
}

/** destructor **/
FTServer::~FTServer() {

	// Trigger thread to stop
	::SetEvent(m_StopThread);

	// Wait until thread finished
	::WaitForSingleObject(m_WaitThread, INFINITE);

	// Close handles
	::CloseHandle(m_StopThread);
	::CloseHandle(m_WaitThread);

	//
	// Free the DLL
	//
	FTClientLib.unload();
	//if(  aDLLHandle != INVALID_HANDLE_VALUE )
	//	FreeLibrary( (HMODULE) aDLLHandle );
	//terminates the QThread and waits for finishing the QThread
	terminate();
	wait();
}

/** QThread run @override **/
void FTServer::run() {

	if (pMemData != NULL) {
		pMemData->data.DataID = 1;
		pMemData->data.CamWidth = 100;
		pMemData->data.CamHeight = 250;
	}

	forever
	{
	    // Check event for stop thread
		if(::WaitForSingleObject(m_StopThread, 0) == WAIT_OBJECT_0)
		{
			// Set event
			::SetEvent(m_WaitThread);
			return;
		}

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
			// Multiply the FaceAPI value, with the sensitivity setting
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
			ReleaseMutex(hFTMutex);
		}

		// just for lower cpu load
		msleep(15);	
		yieldCurrentThread();
		pMemData->data.DataID += 1;
	}
}

//
// Create a memory-mapping to the Freetrack data.
// It contains the tracking data, a handle to the main-window and the program-name of the Game!
//
//
bool FTServer::FTCreateMapping(HANDLE handle)
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
		qDebug() << "FTCreateMapping says: FileMapping Created again:" << hFTMemMap;
		pMemData = (FTMemMap *) MapViewOfFile(hFTMemMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TFreeTrackData) + sizeof(hFTMemMap) + 100);
		if (pMemData != NULL) {
			pMemData->handle = handle;	// The game uses the handle, to send a message that the Program-Name was set!
		}
	    hFTMutex = CreateMutexA(NULL, false, FREETRACK_MUTEX);
	}
	else {
		QMessageBox::information(0, "FaceTrackNoIR error", QString("FTServer Error! \n"));
		return false;
	}

	return true;
}

//
// Destory the FileMapping to the shared memory
//
void FTServer::FTDestroyMapping()
{
	if ( pMemData != NULL ) {
		UnmapViewOfFile ( pMemData );
	}
	
	CloseHandle( hFTMutex );
	CloseHandle( hFTMemMap );
	hFTMemMap = 0;
}

//
// Get the program-name from the client (Game!).
//
QString FTServer::GetProgramName() {   
QString *str;

	str = new QString(pMemData->ProgramName);
	return *str;
}


//
// Check if the Client DLL exists and load it (to test it), if so.
// Returns 'true' if all seems OK.
//
bool FTServer::checkServerInstallationOK( HANDLE handle )
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
			FTClientLib.setFileName(aFileName);
			FTClientLib.load();
			provider = (importProvider) FTClientLib.resolve("FTProvider");
			if (provider) {
				pProvider = provider();
				qDebug() << "FTCheckClientDLL says: Provider =" << pProvider;
			}
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

//END
