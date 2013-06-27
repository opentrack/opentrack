/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2010-2011	Wim Vriend (Developing)								*
*							Ron Hendriks (Researching and Testing)				*
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
* FTNoIR_Protocol: the Class, that communicates headpose-data					*
*				to games, using the SimConnect.dll.		         				*
*				SimConnect.dll is a so called 'side-by-side' assembly, so it	*
*				must be treated as such...										*
********************************************************************************/
/*
	Modifications (last one on top):
	20110401 - WVR: Moved protocol to a DLL, convenient for installation etc.
	20101224 - WVR: Base class is no longer inheriting QThread. sendHeadposeToGame
					is called from run() of Tracker.cpp
*/
#include "ftnoir_protocol_sc.h"
#include "facetracknoir/global-settings.h"

importSimConnect_CameraSetRelative6DOF FTNoIR_Protocol::simconnect_set6DOF;
HANDLE FTNoIR_Protocol::hSimConnect = 0;			// Handle to SimConnect

float FTNoIR_Protocol::virtSCPosX = 0.0f;			// Headpose
float FTNoIR_Protocol::virtSCPosY = 0.0f;
float FTNoIR_Protocol::virtSCPosZ = 0.0f;
	
float FTNoIR_Protocol::virtSCRotX = 0.0f;
float FTNoIR_Protocol::virtSCRotY = 0.0f;
float FTNoIR_Protocol::virtSCRotZ = 0.0f;

float FTNoIR_Protocol::prevSCPosX = 0.0f;			// previous Headpose
float FTNoIR_Protocol::prevSCPosY = 0.0f;
float FTNoIR_Protocol::prevSCPosZ = 0.0f;
	
float FTNoIR_Protocol::prevSCRotX = 0.0f;
float FTNoIR_Protocol::prevSCRotY = 0.0f;
float FTNoIR_Protocol::prevSCRotZ = 0.0f;

/** constructor **/
FTNoIR_Protocol::FTNoIR_Protocol()
{
	ProgramName = "Microsoft FSX";
	blnSimConnectActive = false;
	hSimConnect = 0;
}

/** destructor **/
FTNoIR_Protocol::~FTNoIR_Protocol()
{
	qDebug() << "~FTNoIR_Protocol says: inside" << FTNoIR_Protocol::hSimConnect;

	if (hSimConnect != 0) {
		qDebug() << "~FTNoIR_Protocol says: before simconnect_close";
		if (SUCCEEDED( simconnect_close( FTNoIR_Protocol::hSimConnect ) ) ) {
			qDebug() << "~FTNoIR_Protocol says: close SUCCEEDED";
		}
	}
//	SCClientLib.unload(); Generates crash when tracker is ended...
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Protocol::loadSettings() {
// None yet...
}

//
// Update Headpose in Game.
//
void FTNoIR_Protocol::sendHeadposeToGame( double *headpose, double *rawheadpose ) {
PDWORD_PTR MsgResult = 0;


    virtSCRotX = -headpose[Pitch];					// degrees
    virtSCRotY = -headpose[Yaw];
    virtSCRotZ = headpose[Roll];

    virtSCPosX = headpose[TX]/100.f;						// cm to meters
    virtSCPosY = headpose[TY]/100.f;
    virtSCPosZ = -headpose[TZ]/100.f;

	//
	// It's only useful to send data, if the connection was made.
	//
	if (!blnSimConnectActive) {
        if (SUCCEEDED(simconnect_open(&hSimConnect, "FaceTrackNoIR", NULL, 0, 0, 0))) {
            qDebug() << "FTNoIR_Protocol::sendHeadposeToGame() says: SimConnect active!";

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
//		//
//		// Only do this when the data has changed. This way, the HAT-switch can be used when tracking is OFF.
//		//
//		if ((prevPosX != virtPosX) || (prevPosY != virtPosY) || (prevPosZ != virtPosZ) ||
//			(prevRotX != virtRotX) || (prevRotY != virtRotY) || (prevRotZ != virtRotZ)) {
////			if (S_OK == simconnect_set6DOF(hSimConnect, virtPosX, virtPosY, virtPosZ, virtRotX, virtRotZ, virtRotY)) {
////					qDebug() << "FTNoIR_Protocol::run() says: SimConnect data written!";
////			}
//		}
//
//		prevPosX = virtPosX;
//		prevPosY = virtPosY;
//		prevPosZ = virtPosZ;
//		prevRotX = virtRotX;
//		prevRotY = virtRotY;
//		prevRotZ = virtRotZ;

		if SUCCEEDED(simconnect_calldispatch(hSimConnect, processNextSimconnectEvent, NULL)) {
			qDebug() << "FTNoIR_Protocol::sendHeadposeToGame() says: Dispatching";
		}
		else {
			qDebug() << "FTNoIR_Protocol::sendHeadposeToGame() says: Error Dispatching!";
		}
	}
}

//
// Returns 'true' if all seems OK.
//
bool FTNoIR_Protocol::checkServerInstallationOK()
{   
	qDebug() << "SCCheckClientDLL says: Starting Function";

    const char* simconnect_paths[] = {
        "SimConnect.DLL",
        "C:\\Windows\\WinSxS\\x86_microsoft.flightsimulator.simconnect_67c7c14424d61b5b_10.0.60905.0_none_dd92b94d8a196297\\SimConnect.DLL",
        "C:\\Windows\\WinSxS\\x86_microsoft.flightsimulator.simconnect_67c7c14424d61b5b_10.0.61242.0_none_e079b46b85043c20\\SimConnect.DLL",
        "C:\\Windows\\WinSxS\\x86_microsoft.flightsimulator.simconnect_67c7c14424d61b5b_10.0.61259.0_none_55f5ecdc14f60568\\SimConnect.DLL",
        NULL
    };

    for (int i = 0; simconnect_paths[i]; i++)
    {
        SCClientLib.setFileName(simconnect_paths[i]);
        if (!SCClientLib.load()) {
            qDebug() << "SCCheckClientDLL says: Error loading SimConnect DLL";
            qDebug() << SCClientLib.errorString();
            continue;
        }
        break;
    }

	//
	// Get the functions from the DLL.
	//
	simconnect_open = (importSimConnect_Open) SCClientLib.resolve("SimConnect_Open");
	if (simconnect_open == NULL) {
		qDebug() << "FTNoIR_Protocol::checkServerInstallationOK() says: SimConnect_Open function not found in DLL!";
		return false;
	}
	simconnect_set6DOF = (importSimConnect_CameraSetRelative6DOF) SCClientLib.resolve("SimConnect_CameraSetRelative6DOF");
	if (simconnect_set6DOF == NULL) {
		qDebug() << "FTNoIR_Protocol::checkServerInstallationOK() says: SimConnect_CameraSetRelative6DOF function not found in DLL!";
		return false;
	}
	simconnect_close = (importSimConnect_Close) SCClientLib.resolve("SimConnect_Close");
	if (simconnect_close == NULL) {
		qDebug() << "FTNoIR_Protocol::checkServerInstallationOK() says: SimConnect_Close function not found in DLL!";
		return false;
	}

	//return true;

	simconnect_calldispatch = (importSimConnect_CallDispatch) SCClientLib.resolve("SimConnect_CallDispatch");
	if (simconnect_calldispatch == NULL) {
		qDebug() << "FTNoIR_Protocol::checkServerInstallationOK() says: SimConnect_CallDispatch function not found in DLL!";
		return false;
	}

	simconnect_subscribetosystemevent = (importSimConnect_SubscribeToSystemEvent) SCClientLib.resolve("SimConnect_SubscribeToSystemEvent");
	if (simconnect_subscribetosystemevent == NULL) {
		qDebug() << "FTNoIR_Protocol::checkServerInstallationOK() says: SimConnect_SubscribeToSystemEvent function not found in DLL!";
		return false;
	}

	simconnect_mapclienteventtosimevent = (importSimConnect_MapClientEventToSimEvent) SCClientLib.resolve("SimConnect_MapClientEventToSimEvent");
	if (simconnect_subscribetosystemevent == NULL) {
		qDebug() << "FTNoIR_Protocol::checkServerInstallationOK() says: SimConnect_MapClientEventToSimEvent function not found in DLL!";
		return false;
	}

	simconnect_addclienteventtonotificationgroup = (importSimConnect_AddClientEventToNotificationGroup) SCClientLib.resolve("SimConnect_AddClientEventToNotificationGroup");
	if (simconnect_subscribetosystemevent == NULL) {
		qDebug() << "FTNoIR_Protocol::checkServerInstallationOK() says: SimConnect_AddClientEventToNotificationGroup function not found in DLL!";
		return false;
	}

	simconnect_setnotificationgrouppriority = (importSimConnect_SetNotificationGroupPriority) SCClientLib.resolve("SimConnect_SetNotificationGroupPriority");
	if (simconnect_subscribetosystemevent == NULL) {
		qDebug() << "FTNoIR_Protocol::checkServerInstallationOK() says: SimConnect_SetNotificationGroupPriority function not found in DLL!";
		return false;
	}

	qDebug() << "FTNoIR_Protocol::checkServerInstallationOK() says: SimConnect functions resolved in DLL!";

	return true;
}

void CALLBACK FTNoIR_Protocol::processNextSimconnectEvent(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext)
{
//    HRESULT hr;

    switch(pData->dwID)
    {
        case SIMCONNECT_RECV_ID_EVENT:
        {
            SIMCONNECT_RECV_EVENT *evt = (SIMCONNECT_RECV_EVENT*)pData;

			qDebug() << "FTNoIR_Protocol::processNextSimconnectEvent() says: SimConnect active!";
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
//			qDebug() << "FTNoIR_Protocol::processNextSimconnectEvent() says: Frame event!";
			if ((prevSCPosX != virtSCPosX) || (prevSCPosY != virtSCPosY) || (prevSCPosZ != virtSCPosZ) ||
				(prevSCRotX != virtSCRotX) || (prevSCRotY != virtSCRotY) || (prevSCRotZ != virtSCRotZ)) {
				if (S_OK == simconnect_set6DOF(hSimConnect, virtSCPosX, virtSCPosY, virtSCPosZ, virtSCRotX, virtSCRotZ, virtSCRotY)) {
	//					qDebug() << "FTNoIR_Protocol::run() says: SimConnect data written!";
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
			qDebug() << "FTNoIR_Protocol::processNextSimconnectEvent() says: Quit event!";
//            quit = 1;
            break;
        }

        default:
            break;
    }
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
