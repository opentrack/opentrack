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
* FTServer		FTServer is the Class, that communicates headpose-data			*
*				to games, using the FreeTrackClient.dll.		         		*
********************************************************************************/
#pragma once
#ifndef INCLUDED_FTSERVER_H
#define INCLUDED_FTSERVER_H

#include "ftnoir_protocol_base/ftnoir_protocol_base.h"
#include "ui_ftnoir_ftcontrols.h"
#include "fttypes.h"
#include <QMessageBox>
#include <QSettings>
#include <QLibrary>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <windows.h>
// todo wine glue
//#include "math.h"
#include "facetracknoir/global-settings.h"

typedef char *(WINAPI *importProvider)(void);

class FTNoIR_Protocol : public IProtocol
{
public:
	FTNoIR_Protocol();
	~FTNoIR_Protocol();

	void Release();
    void Initialize();

    bool checkServerInstallationOK();
	void sendHeadposeToGame( THeadPoseData *headpose, THeadPoseData *rawheadpose );
	void getNameFromGame( char *dest );					// Take care dest can handle up to 100 chars...

private:
    bool FTCreateMapping();
	void FTDestroyMapping();

	HANDLE hFTMemMap;
	FTMemMap *pMemData;
	HANDLE hFTMutex;

	__int32 comhandle;									// Handle on x32, command on x64

	// Private properties
	QString ProgramName;
	QLibrary FTClientLib;

	float getRadsFromDegrees ( float degrees ) { return (degrees * 0.017453f); }
	void loadSettings();

};

// Widget that has controls for FTNoIR protocol client-settings.
class FTControls: public QWidget, Ui::UICFTControls, public IProtocolDialog
{
    Q_OBJECT
public:

	explicit FTControls();
    virtual ~FTControls();
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
	Ui::UICFTControls ui;
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

	void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("FreeTrack"); };
	void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("FreeTrack"); };
	void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("FreeTrack protocol"); };

    void getIcon(QIcon *icon) { *icon = QIcon(":/images/freetrack.png"); };
};


#endif//INCLUDED_FTSERVER_H
//END
