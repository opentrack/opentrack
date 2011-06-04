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

#include "..\ftnoir_protocol_base\ftnoir_protocol_base.h"
#include "ui_FTNoIR_FTcontrols.h"
#include "FTTypes.h"
#include <QMessageBox>
#include <QSettings>
#include <QLibrary>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include "Windows.h"
//#include "math.h"

typedef char *(WINAPI *importProvider)(void);

class FTNoIR_Protocol_FT : public IProtocol
{
public:
	FTNoIR_Protocol_FT();
	~FTNoIR_Protocol_FT();

	void Release();
    void Initialize();

	bool checkServerInstallationOK( HANDLE handle );
	void sendHeadposeToGame( T6DOF *headpose );
	void getNameFromGame( char *dest );					// Take care dest can handle up to 100 chars...

private:
	bool FTCreateMapping(HANDLE handle);
	void FTDestroyMapping();

	HANDLE hFTMemMap;
	FTMemMap *pMemData;
	HANDLE hFTMutex;

	HANDLE hMainWindow;									// Save the handle to FaceTrackNoIR main-window
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

private:
	Ui::UICFTControls ui;
	void loadSettings();
	void save();

	/** helper **/
	bool settingsDirty;

private slots:
	void doOK();
	void doCancel();
	void settingChanged() { settingsDirty = true; };
};

#endif//INCLUDED_FTSERVER_H
//END
