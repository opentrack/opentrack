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
#pragma once
#ifndef INCLUDED_FTSERVER_H
#define INCLUDED_FTSERVER_H
 
#include "FTNoIR_cxx_protocolserver.h"
#include "FTTypes.h"
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

typedef char *(WINAPI *importProvider)(void);
typedef bool (WINAPI *importGetData)(TFreeTrackData * data);
typedef HANDLE (WINAPI *importGetMapHandle)(void);

using namespace std;
using namespace v4friend::ftnoir;

class FTServer : public ProtocolServerBase {
	Q_OBJECT

public: 

	// public member methods
	FTServer();
	~FTServer();

	QString GetProgramName();

	// protected member methods
protected:
//	void run();
	bool checkServerInstallationOK( HANDLE handle );
	void sendHeadposeToGame();

private:
	bool FTCreateMapping(HANDLE handle);
	void FTDestroyMapping();

	HANDLE hFTMemMap;
	FTMemMap *pMemData;
	HANDLE hFTMutex;

	/** member varables for saving the head pose **/
	bool confid;

	// Private properties
	QString ProgramName;
	QLibrary FTClientLib;

public:
	void setHeadRotX(float x) { headRotX = getRadsFromDegrees(x); }
	void setHeadRotY(float y) { headRotY = getRadsFromDegrees(y); }
	void setHeadRotZ(float z) { headRotZ = getRadsFromDegrees(z); }
	void setHeadPosX(float x) { headPosX = x * 10; }
	void setHeadPosY(float y) { headPosY = y * 10; }
	void setHeadPosZ(float z) { headPosZ = z * 10; }

	void setVirtRotX(float rot) { virtRotX = getRadsFromDegrees(rot); }
	void setVirtRotY(float rot) { virtRotY = getRadsFromDegrees(rot); }
	void setVirtRotZ(float rot) { virtRotZ = getRadsFromDegrees(rot); }
	void setVirtPosX(float pos) { virtPosX = pos * 10; }
	void setVirtPosY(float pos) { virtPosY = pos * 10; }
	void setVirtPosZ(float pos) { virtPosZ = pos * 10; }

	float getRadsFromDegrees ( float degrees ) { return (degrees * 0.017453f); }

};


#endif//INCLUDED_FTSERVER_H
//END
