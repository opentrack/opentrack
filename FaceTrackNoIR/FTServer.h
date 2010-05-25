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
 
//#include "Windows.h" 
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

class FTServer : public QThread {
	Q_OBJECT

public: 

	// public member methods
	FTServer();
	~FTServer();

	bool FTCreateMapping(HANDLE handle);
	void FTDestroyMapping();
	bool FTCheckClientDLL();
	QString FTGetProgramName();

	// protected member methods
protected:
	void run();

private:
	// Handles to neatly terminate thread...
	HANDLE m_StopThread;
	HANDLE m_WaitThread;

	HANDLE hFTMemMap;
	FTMemMap *pMemData;
	HANDLE hFTMutex;

	/** member varables for saving the head pose **/
	float headPosX;
	float headPosY;
	float headPosZ;
	
	float headRotX;
	float headRotY;
	float headRotZ;
	bool confid;

	// Settings for calculating the Virtual Pose
	float virtPosX;
	float virtPosY;
	float virtPosZ;
	
	float virtRotX;
	float virtRotY;
	float virtRotZ;

	// Private properties
	QString ProgramName;
	QLibrary FTClientLib;

public:
	void setHeadPosX(float x) { headPosX = x; }
	void setHeadPosY(float y) { headPosY = y; }
	void setHeadPosZ(float z) { headPosZ = z; }

	void setHeadRotX(float x) { headRotX = x; }
	void setHeadRotY(float y) { headRotY = y; }
	void setHeadRotZ(float z) { headRotZ = z; }

	void setVirtRotX(float rot) { virtRotX = rot; }
	void setVirtRotY(float rot) { virtRotY = rot; }
	void setVirtRotZ(float rot) { virtRotZ = rot; }
	void setVirtPosX(float pos) { virtPosX = pos; }
	void setVirtPosY(float pos) { virtPosY = pos; }
	void setVirtPosZ(float pos) { virtPosZ = pos; }

};


#endif//INCLUDED_FTSERVER_H
//END
