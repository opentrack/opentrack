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
#pragma once
#ifndef INCLUDED_SCSERVER_H
#define INCLUDED_SCSERVER_H

//
// Prevent the SimConnect manifest from being merged in the application-manifest
// This is necessary to run FaceTrackNoIR on a PC without FSX
//
#define SIMCONNECT_H_NOMANIFEST 
#include "FTNoIR_cxx_protocolserver.h"
//#include "Windows.h" 
//#include <stdlib.h>
#include "SimConnect.h"
#include <QApplication>
#include <QString>
#include <QMessageBox>
#include <QSettings>
#include <QFile>
#include <QDebug>
#include <QLibrary>

typedef HRESULT (WINAPI *importSimConnect_Open)(HANDLE * phSimConnect, LPCSTR szName, HWND hWnd, DWORD UserEventWin32, HANDLE hEventHandle, DWORD ConfigIndex);
typedef HRESULT (WINAPI *importSimConnect_Close)(HANDLE hSimConnect);
typedef HRESULT (WINAPI *importSimConnect_CameraSetRelative6DOF)(HANDLE hSimConnect, float fDeltaX, float fDeltaY, float fDeltaZ, float fPitchDeg, float fBankDeg, float fHeadingDeg);
typedef HRESULT (WINAPI *importSimConnect_CallDispatch)(HANDLE hSimConnect, DispatchProc pfcnDispatch, void * pContext);
typedef HRESULT (WINAPI *importSimConnect_SubscribeToSystemEvent)(HANDLE hSimConnect, SIMCONNECT_CLIENT_EVENT_ID EventID, const char * SystemEventName);
typedef HRESULT (WINAPI *importSimConnect_MapClientEventToSimEvent)(HANDLE hSimConnect, SIMCONNECT_CLIENT_EVENT_ID EventID, const char * EventName);
typedef HRESULT (WINAPI *importSimConnect_AddClientEventToNotificationGroup)(HANDLE hSimConnect, SIMCONNECT_NOTIFICATION_GROUP_ID GroupID, SIMCONNECT_CLIENT_EVENT_ID EventID, BOOL bMaskable);
typedef HRESULT (WINAPI *importSimConnect_SetNotificationGroupPriority)(HANDLE hSimConnect, SIMCONNECT_NOTIFICATION_GROUP_ID GroupID, DWORD uPriority);

using namespace std;
using namespace v4friend::ftnoir;

static const char* SC_CLIENT_FILENAME = "SimConnect.dll";

enum GROUP_ID
{
    GROUP0=0,
};

enum EVENT_ID
{
	EVENT_PING=0,
	EVENT_INIT,
};

enum INPUT_ID
{
    INPUT0=0,
};

class SCServer : public ProtocolServerBase {
	Q_OBJECT

public: 

	// public member methods
	SCServer();
	~SCServer();

	QString GetProgramName();

	// protected member methods
protected:
	bool checkServerInstallationOK( HANDLE handle );
	void sendHeadposeToGame();

private:
	// Private properties
	QString ProgramName;
	QLibrary SCClientLib;

	importSimConnect_Open simconnect_open;							// SimConnect function(s) in DLL
	importSimConnect_Close simconnect_close;
	static importSimConnect_CameraSetRelative6DOF simconnect_set6DOF;
	importSimConnect_CallDispatch simconnect_calldispatch;
	importSimConnect_SubscribeToSystemEvent simconnect_subscribetosystemevent;
	importSimConnect_MapClientEventToSimEvent simconnect_mapclienteventtosimevent;
	importSimConnect_AddClientEventToNotificationGroup simconnect_addclienteventtonotificationgroup;
	importSimConnect_SetNotificationGroupPriority simconnect_setnotificationgrouppriority;

	static HANDLE hSimConnect;										// Handle to SimConnect
	static void CALLBACK processNextSimconnectEvent(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext);

	static float virtSCPosX;
	static float virtSCPosY;
	static float virtSCPosZ;
	
	static float virtSCRotX;
	static float virtSCRotY;
	static float virtSCRotZ;

	static float prevSCPosX;
	static float prevSCPosY;
	static float prevSCPosZ;
	
	static float prevSCRotX;
	static float prevSCRotY;
	static float prevSCRotZ;

	bool blnSimConnectActive;

public:
	void setVirtRotX(float rot) { virtSCRotX = -1.0f * rot; }			// degrees
	void setVirtRotY(float rot) { virtSCRotY = -1.0f * rot; }
	void setVirtRotZ(float rot) { virtSCRotZ = rot; }

	void setVirtPosX(float pos) { virtSCPosX = pos/100.f; }			// cm to meters
	void setVirtPosY(float pos) { virtSCPosY = pos/100.f; }
	void setVirtPosZ(float pos) { virtSCPosZ = -1.0f * pos/100.f; }


};


#endif//INCLUDED_SCSERVER_H
//END
