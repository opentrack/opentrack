/********************************************************************************
* FSUIPCServer		FSUIPCServer is the Class, that communicates headpose-data	*
*					to games, using the FSUIPC.dll.			         			*
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
#ifndef INCLUDED_FSUIPCSERVER_H
#define INCLUDED_FSUIPCSERVER_H

#include "Windows.h" 
#include <stdlib.h>
#include "FTNoIR_cxx_protocolserver.h"
#include "FSUIPC_User.h"
#include <QString>
#include <QMessageBox>
#include <QSettings>
#include <QFile>
#include <QApplication>
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QLibrary>

#include "ui_FTNoIR_FSUIPCcontrols.h"

using namespace std;
using namespace v4friend::ftnoir;

static const char* FSUIPC_FILENAME = "C:\\Program Files\\Microsoft Games\\Flight Simulator 9\\Modules\\FSUIPC.dll";

//
// Define the structures necessary for the FSUIPC_Write calls
//
#pragma pack(push,1)		// All fields in structure must be byte aligned.
typedef struct
{
 int Control;				// Control identifier
 int Value;					// Value of DOF
} TFSState;
#pragma pack(pop)

class FSUIPCServer : public ProtocolServerBase {
	Q_OBJECT

public: 

	// public member methods
	FSUIPCServer();
	~FSUIPCServer();

	// protected member methods
protected:
	bool checkServerInstallationOK();
	void sendHeadposeToGame();

private:
	// Private properties
	QString ProgramName;
	QLibrary FSUIPCLib;
	QString LocationOfDLL;
	void loadSettings();
	float prevPosX, prevPosY, prevPosZ, prevRotX, prevRotY, prevRotZ;
	
public:

	// Settings for calculating the Virtual Pose
	void setVirtRotX(float rot) { virtRotX = -1.0f * rot; }				// degrees
	void setVirtRotY(float rot) { virtRotY = rot; }
	void setVirtRotZ(float rot) { virtRotZ = rot; }

	void setVirtPosX(float pos) { virtPosX = 0.0f; }					// cm, X and Y are not working for FS2002/2004!
	void setVirtPosY(float pos) { virtPosY = 0.0f; }
	void setVirtPosZ(float pos) { virtPosZ = -1.0f * pos; }

	static int scale2AnalogLimits( float x, float min_x, float max_x );
};

// Widget that has controls for FSUIPC server-settings.
class FSUIPCControls: public QWidget, public Ui::UICFSUIPCControls
{
    Q_OBJECT
public:

	explicit FSUIPCControls( QWidget *parent=0, Qt::WindowFlags f=0 );
    virtual ~FSUIPCControls();
	void showEvent ( QShowEvent * event );

private:
	Ui::UICFSUIPCControls ui;
	void loadSettings();
	void save();

	/** helper **/
	bool settingsDirty;

private slots:
	void doOK();
	void doCancel();
	void getLocationOfDLL();

};


#endif//INCLUDED_FSUIPCSERVER_H
//END
