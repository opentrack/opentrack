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
#include "Windows.h" 
#include <stdlib.h>
#include "SimConnect.h"
#include <QString>
#include <QMessageBox>
#include <QSettings>
#include <QFile>
#include <QApplication>
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QLibrary>

typedef HRESULT (WINAPI *importSimConnect_Open)(HANDLE * phSimConnect, LPCSTR szName, HWND hWnd, DWORD UserEventWin32, HANDLE hEventHandle, DWORD ConfigIndex);
typedef HRESULT (WINAPI *importSimConnect_Close)(HANDLE hSimConnect);
typedef HRESULT (WINAPI *importSimConnect_CameraSetRelative6DOF)(HANDLE hSimConnect, float fDeltaX, float fDeltaY, float fDeltaZ, float fPitchDeg, float fBankDeg, float fHeadingDeg);

using namespace std;
using namespace v4friend::ftnoir;

static const char* SC_CLIENT_FILENAME = "SimConnect.dll";

class SCServer : public ProtocolServerBase {
	Q_OBJECT

public: 

	// public member methods
	SCServer();
	~SCServer();

	// protected member methods
protected:
//	void run();
	bool checkServerInstallationOK( HANDLE handle );
	void sendHeadposeToGame();

private:
	// Private properties
	QString ProgramName;
	QLibrary SCClientLib;

public:
	void setVirtRotX(float rot) { virtRotX = -1.0f * rot; }			// degrees
	void setVirtRotY(float rot) { virtRotY = -1.0f * rot; }
	void setVirtRotZ(float rot) { virtRotZ = rot; }

	void setVirtPosX(float pos) { virtPosX = pos/100.f; }			// cm to meters
	void setVirtPosY(float pos) { virtPosY = pos/100.f; }
	void setVirtPosZ(float pos) { virtPosZ = -1.0f * pos/100.f; }
};


#endif//INCLUDED_SCSERVER_H
//END
