/********************************************************************************
* SCServer		SCServer is the Class, that communicates headpose-data	*
*					to games, using the SimConnect.dll.		         			*
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
#include "SCServer.h"

/** constructor **/
SCServer::SCServer() {

	// Create events
	//m_StopThread = CreateEvent(0, TRUE, FALSE, 0);
	//m_WaitThread = CreateEvent(0, TRUE, FALSE, 0);

	ProgramName = "";
}

/** destructor **/
SCServer::~SCServer() {

	// Trigger thread to stop
	//::SetEvent(m_StopThread);

	// Wait until thread finished
	//if (isRunning()) {
	//	::WaitForSingleObject(m_WaitThread, INFINITE);
	//}

	//// Close handles
	//::CloseHandle(m_StopThread);
	//::CloseHandle(m_WaitThread);

	//
	// Free the DLL
	//
	SCClientLib.unload();

	//terminates the QThread and waits for finishing the QThread
	//terminate();
	//wait();
}

/** QThread run @override **/
void SCServer::sendHeadposeToGame() {
bool blnSimConnectActive = false;
HANDLE  hSimConnect = NULL;										// Handle to SimConnect
importSimConnect_Open simconnect_open;							// SimConnect function(s) in DLL
importSimConnect_Close simconnect_close;
importSimConnect_CameraSetRelative6DOF simconnect_set6DOF;

float prevPosX, prevPosY, prevPosZ, prevRotX, prevRotY, prevRotZ;

	//
	// Get the SimConnect_Open function from the DLL and use it!
	//
	simconnect_open = (importSimConnect_Open) SCClientLib.resolve("SimConnect_Open");
	if (simconnect_open == NULL) {
		qDebug() << "SCServer::run() says: SimConnect_Open function not found in DLL!";
		return;
	}
	simconnect_set6DOF = (importSimConnect_CameraSetRelative6DOF) SCClientLib.resolve("SimConnect_CameraSetRelative6DOF");
	if (simconnect_set6DOF == NULL) {
		qDebug() << "SCServer::run() says: SimConnect_CameraSetRelative6DOF function not found in DLL!";
		return;
	}
	simconnect_close = (importSimConnect_Close) SCClientLib.resolve("SimConnect_Close");
	if (simconnect_close == NULL) {
		qDebug() << "SCServer::run() says: SimConnect_Close function not found in DLL!";
		return;
	}

	qDebug() << "SCServer::run() says: SimConnect functions resolved in DLL!";

	prevPosX = 0.0f;
	prevPosY = 0.0f;
	prevPosZ = 0.0f;
	prevRotX = 0.0f;
	prevRotY = 0.0f;
	prevRotZ = 0.0f;

	forever
	{
	    // Check event for stop thread
		//if(::WaitForSingleObject(m_StopThread, 0) == WAIT_OBJECT_0)
		//{
		//	// Set event
//	simconnect_close( hSimConnect );
		//	::SetEvent(m_WaitThread);
		//	return;
		//}

		if (!blnSimConnectActive) {
			if (SUCCEEDED(simconnect_open(&hSimConnect, "FaceTrackNoIR", NULL, 0, 0, 0))) {
				qDebug() << "SCServer::run() says: SimConnect active!";
				blnSimConnectActive = true;
			}
			//else {
			//	msleep(5000);	
			//	yieldCurrentThread();
			//}
		}
		else {
			//
			// Write the 6DOF-data to FSX
			//
			// Only do this when the data has changed. This way, the HAT-switch can be used when tracking is OFF.
			//
			if ((prevPosX != virtPosX) || (prevPosY != virtPosY) || (prevPosZ != virtPosZ) ||
				(prevRotX != virtRotX) || (prevRotY != virtRotY) || (prevRotZ != virtRotZ)) {
				if (S_OK == simconnect_set6DOF(hSimConnect, virtPosX, virtPosY, virtPosZ, virtRotX, virtRotZ, virtRotY)) {
//					qDebug() << "SCServer::run() says: SimConnect data written!";
				}
			}

			prevPosX = virtPosX;
			prevPosY = virtPosY;
			prevPosZ = virtPosZ;
			prevRotX = virtRotX;
			prevRotY = virtRotY;
			prevRotZ = virtRotZ;

			// just for lower cpu load
			//msleep(15);	
			//yieldCurrentThread();
		}
	}
}

//
// Check if the Client DLL exists and load it (to test it), if so.
// Returns 'true' if all seems OK.
//
bool SCServer::checkServerInstallationOK( HANDLE handle )
{   
	QString aFileName;														// File Path and Name

	// Code to activate the context for the SimConnect DLL
	ACTCTX act = { 0 };
	HANDLE hctx;
	ULONG_PTR ulCookie;


	qDebug() << "SCCheckClientDLL says: Starting Function";

	try {

		act.cbSize = sizeof(act);
		act.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID;

		QString manifest(QCoreApplication::applicationDirPath());
		manifest += "\\FaceTrackNoIR.exe";
		const wchar_t * encodedName = reinterpret_cast<const wchar_t *>(manifest.utf16());
		
		act.lpSource = encodedName;
		act.lpResourceName = MAKEINTRESOURCE(101);

		hctx = CreateActCtx (&act);

		if (hctx != INVALID_HANDLE_VALUE) { 
			if (!ActivateActCtx(hctx, &ulCookie)) { 
				ReleaseActCtx(hctx);
				qDebug() << "FTCheckClientDLL says: Error activating SimConnect manifest";
			}
		}
		else {
			qDebug() << "FTCheckClientDLL says: Error INVALID_HANDLE: " << GetLastError();
			return false;
		}

		//
		// Just try to load the DLL. Let Windows handle the PATH's and such trivialities...
		//
		aFileName = SC_CLIENT_FILENAME;
						
		//
		// Load the DLL.
		//
		SCClientLib.setFileName(aFileName);
		if (SCClientLib.load() != true) {
			qDebug() << "FTCheckClientDLL says: Error loading SimConnect DLL";
			return false;
		}

		//
		// Deactivate the context again: the function-references should stay in-tact...
		//
		DeactivateActCtx(0, ulCookie);
		ReleaseActCtx(hctx);

	} catch(...) {
		return false;
	}
	return true;
}

//END
