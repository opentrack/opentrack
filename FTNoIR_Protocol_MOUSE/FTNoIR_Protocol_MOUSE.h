/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2010-2011	Wim Vriend (Developing)								*
*							Ron Hendriks (Researching and Testing)				*
*																				*
* http://facetracknoir.sourceforge.net/home/default.htm							*
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
* FTNoIR_Protocol_Mouse	The Class, that communicates headpose-data by			*
*						generating Mouse commands.								*
*						Many games (like FPS's) support Mouse-look features,	*
*						but no face-tracking.									*
********************************************************************************/
#pragma once
#ifndef INCLUDED_MOUSESERVER_H
#define INCLUDED_MOUSESERVER_H

#include "..\ftnoir_protocol_base\ftnoir_protocol_base.h"
#include "ui_FTNoIR_MOUSEcontrols.h"
#include <QMessageBox>
#include <QSettings>
#include <QLibrary>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include "Windows.h"

#define MOUSE_AXIS_MIN 0
#define MOUSE_AXIS_MAX 65535

enum FTN_AngleName {
	FTN_PITCH = 1,
	FTN_YAW = 2,
	FTN_ROLL = 3,
	FTN_X = 4,
	FTN_Y = 5,
	FTN_Z = 6
};

enum FTN_MouseStyle {
	FTN_ABSOLUTE = 0,
	FTN_RELATIVE = 2	
};


class FTNoIR_Protocol_MOUSE : public IProtocol
{
public:
	FTNoIR_Protocol_MOUSE();
	~FTNoIR_Protocol_MOUSE();

	void Release();
    void Initialize();

	bool checkServerInstallationOK( HANDLE handle );
	void sendHeadposeToGame( T6DOF *headpose );

private:
	HANDLE h;
	INPUT MouseStruct;

	FTN_AngleName Mouse_X;			// Map one of the 6DOF's to this Mouse direction
	FTN_AngleName Mouse_Y;
	FTN_AngleName Mouse_Wheel;
	FTN_MouseStyle Mouse_Style;		// AutoPan, Absolute or Relative?
	bool useVirtualDesk;			// Extend the mouse-range beyond the standard
	float prev_fMouse_X;			// The previous value(s)
	float mouse_X_factor;			// Sensitivity factor
	float prev_fMouse_Y;
	float mouse_Y_factor;			// Sensitivity factor
	float prev_fMouse_Wheel;
	float mouse_Wheel_factor;		// Sensitivity factor

	long scale2AnalogLimits( float x, float min_x, float max_x );
	void loadSettings();

};

// Widget that has controls for FTNoIR protocol client-settings.
class MOUSEControls: public QWidget, Ui::UICMOUSEControls, public IProtocolDialog
{
    Q_OBJECT
public:

	explicit MOUSEControls();
    virtual ~MOUSEControls();
	void showEvent ( QShowEvent * event );

	void Release();											// Member functions which are accessible from outside the DLL
    void Initialize(QWidget *parent);

private:
	Ui::UICMOUSEControls ui;
	void loadSettings();
	void save();

	/** helper **/
	bool settingsDirty;

private slots:
	void doOK();
	void doCancel();
	void settingChanged( int setting ) { settingsDirty = true; };
};

#endif//INCLUDED_MOUSESERVER_H
//END
