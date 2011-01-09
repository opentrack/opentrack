/********************************************************************************
* SCServer		SCServer is the Class, that communicates headpose-data			*
*				to games, using the SimConnect.dll.		         				*
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
	20101224 - WVR: Base class is no longer inheriting QThread. sendHeadposeToGame
					is called from run() of Tracker.cpp
*/
#include "SCServer.h"

importSimConnect_CameraSetRelative6DOF SCServer::simconnect_set6DOF;
HANDLE SCServer::hSimConnect = 0;			// Handle to SimConnect

float SCServer::virtSCPosX = 0.0f;			// Headpose
float SCServer::virtSCPosY = 0.0f;
float SCServer::virtSCPosZ = 0.0f;
	
float SCServer::virtSCRotX = 0.0f;
float SCServer::virtSCRotY = 0.0f;
float SCServer::virtSCRotZ = 0.0f;

float SCServer::prevSCPosX = 0.0f;			// previous Headpose
float SCServer::prevSCPosY = 0.0f;
float SCServer::prevSCPosZ = 0.0f;
	
float SCServer::prevSCRotX = 0.0f;
float SCServer::prevSCRotY = 0.0f;
float SCServer::prevSCRotZ = 0.0f;

/** constructor **/
SCServer::SCServer() {
	ProgramName = "Microsoft FSX";
	blnSimConnectActive = false;
	hSimConnect = 0;
}

/** destructor **/
SCServer::~SCServer() {

	qDebug() << "~SCServer says: inside" << SCServer::hSimConnect;

	if (hSimConnect != 0) {
		qDebug() << "~SCServer says: before simconnect_close";
		if (SUCCEEDED( simconnect_close( SCServer::hSimConnect ) ) ) {
			qDebug() << "~SCServer says: close SUCCEEDED";
		}
	}

	qDebug() << "~SCServer says: before unload";
//	SCClientLib.unload(); Generates crash when tracker is ended...
	qDebug() << "~SCServer says: finished";
}

//
// Update Headpose in Game.
//
void SCServer::sendHeadposeToGame() {

	//
	// It's only useful to send data, if the connection was made.
	//
	if (!blnSimConnectActive) {
		if (SUCCEEDED(simconnect_open(&hSimConnect, "FaceTrackNoIR", NULL, 0, 0, 0))) {
			qDebug() << "SCServer::sendHeadposeToGame() says: SimConnect active!";

			//set up the events we want to listen for
			HRESULT hr;

			simconnect_subscribetosystemevent(hSimConnect, EVENT_PING, "Frame"); 

			hr = simconnect_mapclienteventtosimevent(hSimConnect, EVENT_INIT, "");
			hr = simconnect_addclienteventtonotificationgroup(hSimConnect, GROUP0, EVENT_INIT, false);
			hr = simconnect_setnotificationgrouppriority(hSimConnect, GROUP0, SIMCONNECT_GROUP_PRIORITY_HIGHEST);
			////hr = SimConnect_MapInputEventToClientEvent(hSimConnect, INPUT0, "VK_COMMA", EVENT_INIT);
			////hr = SimConnect_SetInputGroupState(hSimConnect, INPUT0, SIMCONNECT_STATE_ON);

			blnSimConnectActive = true;
		}
	}
	else {
		//
		// Write the 6DOF-data to FSX
		//
		// Only do this when the data has changed. This way, the HAT-switch can be used when tracking is OFF.
		//
		if ((prevPosX != virtPosX) || (prevPosY != virtPosY) || (prevPosZ != virtPosZ) ||
			(prevRotX != virtRotX) || (prevRotY != virtRotY) || (prevRotZ != virtRotZ)) {
//			if (S_OK == simconnect_set6DOF(hSimConnect, virtPosX, virtPosY, virtPosZ, virtRotX, virtRotZ, virtRotY)) {
//					qDebug() << "SCServer::run() says: SimConnect data written!";
//			}
		}

		prevPosX = virtPosX;
		prevPosY = virtPosY;
		prevPosZ = virtPosZ;
		prevRotX = virtRotX;
		prevRotY = virtRotY;
		prevRotZ = virtRotZ;

		simconnect_calldispatch(hSimConnect, processNextSimconnectEvent, NULL);
	}
}

//
// Check if the Client DLL exists and load it (to test it), if so.
// SimConnect uses a 'side-by-side' installation. The manifest is loaded and (if that's OK) the
// right DLL will be loaded automatically...
//
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

	//
	// Get the functions from the DLL.
	//
	simconnect_open = (importSimConnect_Open) SCClientLib.resolve("SimConnect_Open");
	if (simconnect_open == NULL) {
		qDebug() << "SCServer::checkServerInstallationOK() says: SimConnect_Open function not found in DLL!";
		return false;
	}
	simconnect_set6DOF = (importSimConnect_CameraSetRelative6DOF) SCClientLib.resolve("SimConnect_CameraSetRelative6DOF");
	if (simconnect_set6DOF == NULL) {
		qDebug() << "SCServer::checkServerInstallationOK() says: SimConnect_CameraSetRelative6DOF function not found in DLL!";
		return false;
	}
	simconnect_close = (importSimConnect_Close) SCClientLib.resolve("SimConnect_Close");
	if (simconnect_close == NULL) {
		qDebug() << "SCServer::checkServerInstallationOK() says: SimConnect_Close function not found in DLL!";
		return false;
	}

	//return true;

	simconnect_calldispatch = (importSimConnect_CallDispatch) SCClientLib.resolve("SimConnect_CallDispatch");
	if (simconnect_calldispatch == NULL) {
		qDebug() << "SCServer::checkServerInstallationOK() says: SimConnect_CallDispatch function not found in DLL!";
		return false;
	}

	simconnect_subscribetosystemevent = (importSimConnect_SubscribeToSystemEvent) SCClientLib.resolve("SimConnect_SubscribeToSystemEvent");
	if (simconnect_subscribetosystemevent == NULL) {
		qDebug() << "SCServer::checkServerInstallationOK() says: SimConnect_SubscribeToSystemEvent function not found in DLL!";
		return false;
	}

	simconnect_mapclienteventtosimevent = (importSimConnect_MapClientEventToSimEvent) SCClientLib.resolve("SimConnect_MapClientEventToSimEvent");
	if (simconnect_subscribetosystemevent == NULL) {
		qDebug() << "SCServer::checkServerInstallationOK() says: SimConnect_MapClientEventToSimEvent function not found in DLL!";
		return false;
	}

	simconnect_addclienteventtonotificationgroup = (importSimConnect_AddClientEventToNotificationGroup) SCClientLib.resolve("SimConnect_AddClientEventToNotificationGroup");
	if (simconnect_subscribetosystemevent == NULL) {
		qDebug() << "SCServer::checkServerInstallationOK() says: SimConnect_AddClientEventToNotificationGroup function not found in DLL!";
		return false;
	}

	simconnect_setnotificationgrouppriority = (importSimConnect_SetNotificationGroupPriority) SCClientLib.resolve("SimConnect_SetNotificationGroupPriority");
	if (simconnect_subscribetosystemevent == NULL) {
		qDebug() << "SCServer::checkServerInstallationOK() says: SimConnect_SetNotificationGroupPriority function not found in DLL!";
		return false;
	}

	qDebug() << "SCServer::checkServerInstallationOK() says: SimConnect functions resolved in DLL!";

	return true;
}

//
// Get the program-name from the client (Game!).
//
QString SCServer::GetProgramName() {   
QString *str;

	str = new QString("FSX");
	return *str;
}


void CALLBACK SCServer::processNextSimconnectEvent(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext)
{
//    HRESULT hr;

    switch(pData->dwID)
    {
        case SIMCONNECT_RECV_ID_EVENT:
        {
            SIMCONNECT_RECV_EVENT *evt = (SIMCONNECT_RECV_EVENT*)pData;

			qDebug() << "SCServer::processNextSimconnectEvent() says: SimConnect active!";
            //switch(evt->uEventID)
            //{
            //    //case EVENT_CAMERA_RIGHT:

            //    //    cameraBank = normalize180( cameraBank + 5.0f);

            //    //    hr = SimConnect_CameraSetRelative6DOF(hSimConnect, 0.0f, 0.0f, 0.0f,
            //    //            SIMCONNECT_CAMERA_IGNORE_FIELD,SIMCONNECT_CAMERA_IGNORE_FIELD, cameraBank);

            //    //    printf("\nCamera Bank = %f", cameraBank);
            //    //    break;

            //    //case EVENT_CAMERA_LEFT:
            //    //    
            //    //    cameraBank = normalize180( cameraBank - 5.0f);

            //    //    hr = SimConnect_CameraSetRelative6DOF(hSimConnect, 0.0f, 0.0f, 0.0f,
            //    //            SIMCONNECT_CAMERA_IGNORE_FIELD,SIMCONNECT_CAMERA_IGNORE_FIELD, cameraBank);
            //    //    
            //    //    printf("\nCamera Bank = %f", cameraBank);
            //    //    break;

            //    //default:
            //    //    break;
            //}
            //break;
        }
		case SIMCONNECT_RECV_ID_EVENT_FRAME:
		{
//			qDebug() << "SCServer::processNextSimconnectEvent() says: Frame event!";
			if ((prevSCPosX != virtSCPosX) || (prevSCPosY != virtSCPosY) || (prevSCPosZ != virtSCPosZ) ||
				(prevSCRotX != virtSCRotX) || (prevSCRotY != virtSCRotY) || (prevSCRotZ != virtSCRotZ)) {
				if (S_OK == simconnect_set6DOF(hSimConnect, virtSCPosX, virtSCPosY, virtSCPosZ, virtSCRotX, virtSCRotZ, virtSCRotY)) {
	//					qDebug() << "SCServer::run() says: SimConnect data written!";
				}
			}
			prevSCPosX = virtSCPosX;
			prevSCPosY = virtSCPosY;
			prevSCPosZ = virtSCPosZ;
			prevSCRotX = virtSCRotX;
			prevSCRotY = virtSCRotY;
			prevSCRotZ = virtSCRotZ;
		}

        case SIMCONNECT_RECV_ID_EXCEPTION:
        {
            SIMCONNECT_RECV_EXCEPTION *except = (SIMCONNECT_RECV_EXCEPTION*)pData;
            
            switch (except->dwException)
            {
            case SIMCONNECT_EXCEPTION_ERROR:
                printf("\nCamera error");
                break;

            default:
                printf("\nException");
                break;
            }
            break;
        }

        case SIMCONNECT_RECV_ID_QUIT:
        {
			qDebug() << "SCServer::processNextSimconnectEvent() says: Quit event!";
//            quit = 1;
            break;
        }

        default:
            break;
    }
}


//END
