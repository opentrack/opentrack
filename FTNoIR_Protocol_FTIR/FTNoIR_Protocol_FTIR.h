/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2010	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
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
* FTIRServer		FTIRServer is the Class, that communicates headpose-data	*
*					to games, using the NPClient.dll.		         			*
********************************************************************************/
#pragma once
#ifndef INCLUDED_FTIRSERVER_H
#define INCLUDED_FTIRSERVER_H

#include "..\ftnoir_protocol_base\ftnoir_protocol_base.h"
#include "ui_FTNoIR_FTIRcontrols.h"
#include "FTIRTypes.h"
#include <QMessageBox>
#include <QSettings>
#include <QLibrary>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include "Windows.h"
//#include "math.h"

//typedef void (WINAPI *importSetPosition)(float, float, float, float, float, float);
typedef int (WINAPI *importSetData)(TRACKIRDATA*);
typedef void (WINAPI *importTIRViewsStart)(void);
typedef void (WINAPI *importTIRViewsStop)(void);


class FTNoIR_Protocol : public IProtocol
{
public:
	FTNoIR_Protocol();
	~FTNoIR_Protocol();

	void Release();
    void Initialize();

	bool checkServerInstallationOK( HANDLE handle );
	void sendHeadposeToGame( THeadPoseData *headpose, THeadPoseData *rawheadpose );
	void getNameFromGame( char *dest );					// Take care dest can handle up to 100 chars...

private:
	bool FTIRCreateMapping(HANDLE handle);
	void FTIRDestroyMapping();

//	importSetPosition setposition;						// Function inside NPClient.dll (old style)
	importSetData setdata;								// Function inside NPClient.dll
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
	bool useDummyExe;

	float scale2AnalogLimits( float x, float min_x, float max_x );
	void loadSettings();

};

// Widget that has controls for FTNoIR protocol client-settings.
class FTIRControls: public QWidget, Ui::UICFTIRControls, public IProtocolDialog
{
    Q_OBJECT
public:

	explicit FTIRControls();
    virtual ~FTIRControls();
	void showEvent ( QShowEvent * event );

	void Release();											// Member functions which are accessible from outside the DLL
    void Initialize(QWidget *parent);
	void registerProtocol(IProtocol *protocol) {
		theProtocol = (FTNoIR_Protocol *) protocol;			// Accept the pointer to the Protocol
	};
	void unRegisterProtocol() {
		theProtocol = NULL;									// Reset the pointer
	};

private:
	Ui::UICFTIRControls ui;
	void loadSettings();
	void save();

	/** helper **/
	bool settingsDirty;
	FTNoIR_Protocol *theProtocol;

private slots:
	void doOK();
	void doCancel();
	void chkTIRViewsChanged() { settingsDirty = true; };
	void settingChanged() { settingsDirty = true; };
};

//*******************************************************************************************************
// FaceTrackNoIR Protocol DLL. Functions used to get general info on the Protocol
//*******************************************************************************************************
class FTNoIR_ProtocolDll : public IProtocolDll
{
public:
	FTNoIR_ProtocolDll();
	~FTNoIR_ProtocolDll();

	void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("fake TrackIR"); };
	void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("FTIR"); };
	void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("TrackIR V4 protocol"); };

	void getIcon(QIcon *icon) { *icon = QIcon(":/images/TrackIR.ico"); };
};

#endif//INCLUDED_FTIRSERVER_H
//END
