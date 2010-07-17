/********************************************************************************
* FTIRServer		FTIRServer is the Class, that communicates headpose-data	*
*					to games, using the NPClient.dll.		         			*
*																				*
* Copyright (C) 2010	Wim Vriend (Developing)									*
*						Ron Hendriks (Testing and Research)						*
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
/*
	Modifications (last one on top):
*/
#include "FTIRServer.h"


float FTIRServer::virtPosX = 0.0f;
float FTIRServer::virtPosY = 0.0f;
float FTIRServer::virtPosZ = 0.0f;

float FTIRServer::virtRotX = 0.0f;
float FTIRServer::virtRotY = 0.0f;
float FTIRServer::virtRotZ = 0.0f;

/** constructor **/
FTIRServer::FTIRServer() {

	// Create events
	m_StopThread = CreateEvent(0, TRUE, FALSE, 0);
	m_WaitThread = CreateEvent(0, TRUE, FALSE, 0);

	ProgramName = "";
}

/** destructor **/
FTIRServer::~FTIRServer() {

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
	FTIRClientLib.unload();

	//terminates the QThread and waits for finishing the QThread
	terminate();
	wait();
}

/** QThread run @override **/
void FTIRServer::run() {
	importSetPosition setposition;

	//
	// Get the setposition function from the DLL and use it!
	//
	setposition = (importSetPosition) FTIRClientLib.resolve("SetPosition");
	if (setposition == NULL) {
		qDebug() << "FTIRServer::run() says: SetPosition function not found in DLL!";
		return;
	}
	else {
		setposition (7.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f);
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

		if ( (pMemData != NULL) && (WaitForSingleObject(hFTIRMutex, 100) == WAIT_OBJECT_0) ) {

//			qDebug() << "FTIRServer says: virtRotX =" << virtRotX << " virtRotY =" << virtRotY;
			setposition (virtPosX, virtPosY, virtPosZ, virtRotZ, virtRotX, virtRotY );

			//qDebug() << "FTIRServer says: pMemData.xRot =" << pMemData->data.xRot << " yRot =" << pMemData->data.yRot;
			ReleaseMutex(hFTIRMutex);
		}

		// just for lower cpu load
		msleep(15);	
		yieldCurrentThread();
	}
}

//
// Create a memory-mapping to the Freetrack data.
// It contains the tracking data, a handle to the main-window and the program-name of the Game!
//
//
bool FTIRServer::FTIRCreateMapping(HANDLE handle)
{
	bool result;

	qDebug() << "FTIRCreateMapping says: Starting Function";

	//
	// A FileMapping is used to create 'shared memory' between the FTIRServer and the FTClient.
	//
	// Try to create a FileMapping to the Shared Memory.
	// If one already exists: close it.
	//
	hFTIRMemMap = CreateFileMappingA( INVALID_HANDLE_VALUE , 00 , PAGE_READWRITE , 0 , 
		                           sizeof( TRACKIRDATA ) + sizeof( HANDLE ) + 100, 
								   (LPCSTR) FTIR_MM_DATA );

	if ( hFTIRMemMap != 0 ) {
		qDebug() << "FTIRCreateMapping says: FileMapping Created!" << hFTIRMemMap;
	}

	if ( ( hFTIRMemMap != 0 ) && ( (long) GetLastError == ERROR_ALREADY_EXISTS ) ) {
		CloseHandle( hFTIRMemMap );
		hFTIRMemMap = 0;
	}

	//
	// Create a new FileMapping, Read/Write access
	//
	hFTIRMemMap = OpenFileMappingA( FILE_MAP_ALL_ACCESS , false , (LPCSTR) FTIR_MM_DATA );
	if ( ( hFTIRMemMap != 0 ) ) {
		qDebug() << "FTIRCreateMapping says: FileMapping Created again:" << hFTIRMemMap;
		pMemData = (FTIRMemMap *) MapViewOfFile(hFTIRMemMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TRACKIRDATA) + sizeof(hFTIRMemMap) + 100);
		if (pMemData != NULL) {
			pMemData->RegisteredHandle = handle;	// The game uses the handle, to send a message that the Program-Name was set!
		}
	    hFTIRMutex = CreateMutexA(NULL, false, FTIR_MUTEX);
	}
	else {
		QMessageBox::information(0, "FaceTrackNoIR error", QString("FTIRServer Error! \n"));
		return false;
	}

  result = false;
return result;
}

//
// Destory the FileMapping to the shared memory
//
void FTIRServer::FTIRDestroyMapping()
{
	if ( pMemData != NULL ) {
		UnmapViewOfFile ( pMemData );
	}
	
	CloseHandle( hFTIRMutex );
	CloseHandle( hFTIRMemMap );
	hFTIRMemMap = 0;
}

//
// Check if the Client DLL exists and load it (to test it), if so.
// Returns 'true' if all seems OK.
//
bool FTIRServer::FTIRCheckClientDLL()
{   
	QSettings settings("NaturalPoint", "NATURALPOINT\\NPClient Location");	// Registry settings (in HK_USER)
	QString aLocation;														// Location of Client DLL
	QString aFileName;														// File Path and Name

	//importProvider provider;
	//char *pProvider;

	qDebug() << "FTCheckClientDLL says: Starting Function";

	try {

		//
		// Load the NPClient.dll from the current path of FaceTrackNoIR, because there is no
		// guarantee TrackIR or GlovePIE is also installed.
		//
		// Write this path in the registry (under NaturalPoint/NATURALPOINT, for the game(s).
		//
		aLocation =  QCoreApplication::applicationDirPath() + "/";
		qDebug() << "FTCheckClientDLL says: Location of DLL =" << aLocation;

		//
		// Append a '/' to the Path and then the name of the dll.
		//
		aFileName = aLocation;
		aFileName.append(FTIR_CLIENT_FILENAME);
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
			FTIRClientLib.setFileName(aFileName);
			FTIRClientLib.load();
			//provider = (importProvider) FTIRClientLib.resolve("FTProvider");
			//if (provider) {
			//	pProvider = provider();
			//	qDebug() << "FTCheckClientDLL says: Provider =" << pProvider;
			//}
		}
		else {
			QMessageBox::information(0, "FaceTrackNoIR error", QString("Necessary file (NPClient.dll) was NOT found!\n"));
			return false;
		}
	} catch(...) {
		settings.~QSettings();
	}
	return true;
}

//
// Scale the measured value to the Joystick values
//
float FTIRServer::scale2AnalogLimits( float x, float min_x, float max_x ) {
double y;
double local_x;
	
	local_x = x;
	if (local_x > max_x) {
		local_x = max_x;
	}
	if (local_x < min_x) {
		local_x = min_x;
	}
	y = ( NP_AXIS_MAX * local_x ) / max_x;

	return (float) y;
}

//END
