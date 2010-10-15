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
#include "Windows.h" 
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
#include <QUdpSocket>

typedef HRESULT (WINAPI *importSimConnect_Open)(HANDLE * phSimConnect, LPCSTR szName, HWND hWnd, DWORD UserEventWin32, HANDLE hEventHandle, DWORD ConfigIndex);
typedef HRESULT (WINAPI *importSimConnect_Close)(HANDLE hSimConnect);
typedef HRESULT (WINAPI *importSimConnect_CameraSetRelative6DOF)(HANDLE hSimConnect, float fDeltaX, float fDeltaY, float fDeltaZ, float fPitchDeg, float fBankDeg, float fHeadingDeg);

using namespace std;

static const char* SC_CLIENT_FILENAME = "SimConnect.dll";

class SCServer : public QThread {
	Q_OBJECT

public: 

	// public member methods
	SCServer();
	~SCServer();

	bool SCCheckClientDLL();

	// protected member methods
protected:
	void run();

private:
	// Handles to neatly terminate thread...
	HANDLE m_StopThread;
	HANDLE m_WaitThread;

	// Private properties
	QString ProgramName;
	QLibrary SCClientLib;

public:

	// Settings for calculating the Virtual Pose
	static float virtPosX;
	static float virtPosY;
	static float virtPosZ;
	
	static float virtRotX;
	static float virtRotY;
	static float virtRotZ;

	static void setVirtRotX(float rot) { virtRotX = rot; }			// degrees
	static void setVirtRotY(float rot) { virtRotY = rot; }
	static void setVirtRotZ(float rot) { virtRotZ = rot; }

	static void setVirtPosX(float pos) { virtPosX = pos/100.f; }	// cm to meters
	static void setVirtPosY(float pos) { virtPosY = pos/100.f; }
	static void setVirtPosZ(float pos) { virtPosZ = pos/100.f; }

};


#endif//INCLUDED_SCSERVER_H
//END
