/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
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
* SCServer		SCServer is the Class, that communicates headpose-data			*
*				to games, using the SimConnect.dll.		         				*
*				SimConnect.dll is a so called 'side-by-side' assembly, so it	*
*				must be treated as such...										*
********************************************************************************/
#pragma once
#ifndef INCLUDED_SCSERVER_H
#define INCLUDED_SCSERVER_H
#include "facetracknoir/global-settings.h"

//
// Prevent the SimConnect manifest from being merged in the application-manifest
// This is necessary to run FaceTrackNoIR on a PC without FSX
//
#define SIMCONNECT_H_NOMANIFEST 
#define _WIN32_WINNT 0x0502

#include "Windows.h"
#include "SimConnect.h"

#include "..\ftnoir_protocol_base\ftnoir_protocol_base.h"
#include "ui_FTNoIR_SCcontrols.h"
#include <QMessageBox>
#include <QSettings>
#include <QLibrary>
#include <QProcess>
#include <QDebug>
#include <QFile>
//#include "math.h"

typedef HRESULT (WINAPI *importSimConnect_Open)(HANDLE * phSimConnect, LPCSTR szName, HWND hWnd, DWORD UserEventWin32, HANDLE hEventHandle, DWORD ConfigIndex);
typedef HRESULT (WINAPI *importSimConnect_Close)(HANDLE hSimConnect);
typedef HRESULT (WINAPI *importSimConnect_CameraSetRelative6DOF)(HANDLE hSimConnect, float fDeltaX, float fDeltaY, float fDeltaZ, float fPitchDeg, float fBankDeg, float fHeadingDeg);
typedef HRESULT (WINAPI *importSimConnect_CallDispatch)(HANDLE hSimConnect, DispatchProc pfcnDispatch, void * pContext);
typedef HRESULT (WINAPI *importSimConnect_SubscribeToSystemEvent)(HANDLE hSimConnect, SIMCONNECT_CLIENT_EVENT_ID EventID, const char * SystemEventName);
typedef HRESULT (WINAPI *importSimConnect_MapClientEventToSimEvent)(HANDLE hSimConnect, SIMCONNECT_CLIENT_EVENT_ID EventID, const char * EventName);
typedef HRESULT (WINAPI *importSimConnect_AddClientEventToNotificationGroup)(HANDLE hSimConnect, SIMCONNECT_NOTIFICATION_GROUP_ID GroupID, SIMCONNECT_CLIENT_EVENT_ID EventID, BOOL bMaskable);
typedef HRESULT (WINAPI *importSimConnect_SetNotificationGroupPriority)(HANDLE hSimConnect, SIMCONNECT_NOTIFICATION_GROUP_ID GroupID, DWORD uPriority);

#define SC_CLIENT_FILENAME "SimConnect.dll"

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

class FTNoIR_Protocol : public IProtocol
{
public:
	FTNoIR_Protocol();
	~FTNoIR_Protocol();
    void Initialize() {}
    bool checkServerInstallationOK();
    void sendHeadposeToGame( double *headpose, double *rawheadpose );
    QString getGameName() {
        return "FS2004/FSX";
    }

private:
	// Private properties
	QString ProgramName;
	QLibrary SCClientLib;

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
	void loadSettings();
};

// Widget that has controls for FTNoIR protocol client-settings.
class SCControls: public QWidget, public IProtocolDialog
{
    Q_OBJECT
public:

	explicit SCControls();
    virtual ~SCControls();
	void showEvent ( QShowEvent * event );

	void Release();											// Member functions which are accessible from outside the DLL
    void Initialize(QWidget *parent);
	void registerProtocol(IProtocol *protocol) {
		theProtocol = (FTNoIR_Protocol *) protocol;			// Accept the pointer to the Protocol
	}
	void unRegisterProtocol() {
		theProtocol = NULL;									// Reset the pointer
	}

private:
	Ui::UICSCControls ui;
	void loadSettings();
	void save();

	/** helper **/
	bool settingsDirty;
	FTNoIR_Protocol *theProtocol;

private slots:
	void doOK();
	void doCancel();
	void settingChanged() { settingsDirty = true; };
};

//*******************************************************************************************************
// FaceTrackNoIR Protocol DLL. Functions used to get general info on the Protocol
//*******************************************************************************************************
class FTNoIR_ProtocolDll : public Metadata
{
public:
	FTNoIR_ProtocolDll();
	~FTNoIR_ProtocolDll();

	void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("FSX SimConnect"); };
	void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("SimConnect"); };
	void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("Microsoft SimConnect protocol"); };

    void getIcon(QIcon *icon) { *icon = QIcon(":/images/fsx.png"); };
};

#endif//INCLUDED_SCSERVER_H
//END
