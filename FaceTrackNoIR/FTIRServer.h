/********************************************************************************
* FTIRServer		FTIRServer is the Class, that communicates headpose-data	*
*					to games, using the NPClient.dll.		         			*
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
#ifndef INCLUDED_FTIRSERVER_H
#define INCLUDED_FTIRSERVER_H
 
#include "FTNoIR_cxx_protocolserver.h"
#include "FTIRTypes.h"
#include <QString>
#include <QMessageBox>
#include <QSettings>
#include <QFile>
#include <QApplication>
#include <QDebug>
#include <QMutex>
#include <QLibrary>
#include <QProcess>

typedef void (WINAPI *importSetPosition)(float x, float y, float z, float xRot, float yRot, float zRot);
typedef void (WINAPI *importTIRViewsStart)(void);
typedef void (WINAPI *importTIRViewsStop)(void);

#include "ui_FTNoIR_FTIRcontrols.h"

using namespace std;
using namespace v4friend::ftnoir;

class FTIRServer : public ProtocolServerBase {
	Q_OBJECT

public: 

	// public member methods
	FTIRServer();
	~FTIRServer();
	QString GetProgramName();

	// protected member methods
protected:
	bool checkServerInstallationOK( HANDLE handle );
	void sendHeadposeToGame();
	void stopServer();

private:
	bool FTIRCreateMapping(HANDLE handle);
	void FTIRDestroyMapping();

	importSetPosition setposition;						// Function inside NPClient.dll
	importTIRViewsStart viewsStart;						// Functions inside TIRViews.dll
	importTIRViewsStop viewsStop;

	HANDLE hFTIRMemMap;
	FTIRMemMap *pMemData;
	HANDLE hFTIRMutex;

	// Private properties
	QString ProgramName;
	QLibrary FTIRClientLib;
	QLibrary FTIRViewsLib;
	QProcess *dummyTrackIR;
	bool useTIRViews;

	float scale2AnalogLimits( float x, float min_x, float max_x );
	void loadSettings();

public:
	void setVirtRotX(float rot) { virtRotX = scale2AnalogLimits (rot, -180.0f, 180.0f); }
	void setVirtRotY(float rot) { virtRotY = scale2AnalogLimits (rot, -180.0f, 180.0f); }
	void setVirtRotZ(float rot) { virtRotZ = scale2AnalogLimits (rot, -180.0f, 180.0f); }

	void setVirtPosX(float pos) { virtPosX = scale2AnalogLimits (pos * 10.0f, -500.0f, 500.0f); }
	void setVirtPosY(float pos) { virtPosY = scale2AnalogLimits (pos * 10.0f, -500.0f, 500.0f); }
	void setVirtPosZ(float pos) { virtPosZ = scale2AnalogLimits (pos * 10.0f, -500.0f, 500.0f); }
};

// Widget that has controls for FTIR server-settings.
class FTIRControls: public QWidget, public Ui::UICFTIRControls
{
    Q_OBJECT
public:

	explicit FTIRControls( QWidget *parent=0, Qt::WindowFlags f=0 );
    virtual ~FTIRControls();
	void showEvent ( QShowEvent * event );

private:
	Ui::UICFTIRControls ui;
	void loadSettings();
	void save();

	/** helper **/
	bool settingsDirty;

private slots:
	void doOK();
	void doCancel();
	void chkTIRViewsChanged() { settingsDirty = true; };
};


#endif//INCLUDED_FTIRSERVER_H
//END
