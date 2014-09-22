/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2013	Wim Vriend (Developing)									*
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
* FGServer			FGServer is the Class, that communicates headpose-data		*
*					to FlightGear, using UDP.				         			*
*					It is based on the (Linux) example made by Melchior FRANZ.	*
********************************************************************************/
#pragma once
#include "ui_ftnoir_vjoy_controls.h"
#include <cmath>
#include "facetracknoir/plugin-api.hpp"

#define FT_PROGRAMID "FT_ProgramID"

class FTNoIR_Protocol : public IProtocol
{
public:
	FTNoIR_Protocol();
    ~FTNoIR_Protocol() override;
    bool checkServerInstallationOK() {
        return true;
    }
    void sendHeadposeToGame( const double *headpose );
    QString getGameName() {
        return "Virtual joystick";
    }
private:
};

// Widget that has controls for FTNoIR protocol client-settings.
class VJoyControls: public QWidget, public IProtocolDialog
{
    Q_OBJECT
public:

	explicit VJoyControls();
    void registerProtocol(IProtocol *) {}
	void unRegisterProtocol() {}

private:
	Ui::UICVJoyControls ui;
	void save();

private slots:
	void doOK();
	void doCancel();
};

class FTNoIR_ProtocolDll : public Metadata
{
public:
	FTNoIR_ProtocolDll();
	~FTNoIR_ProtocolDll();

	void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("VJoy"); }
	void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("VJoy"); }
	void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("VJoy"); }

    void getIcon(QIcon *icon) { *icon = QIcon(":/images/vjoy.png"); }
};

#define VJOY_AXIS_MIN   -32768
#define VJOY_AXIS_NIL   0
#define VJOY_AXIS_MAX   32767

#include <windows.h>

#include <pshpack1.h>

typedef struct _JOYSTICK_STATE
{
        UCHAR ReportId;                         // Report Id
        SHORT XAxis;                            // X Axis
        SHORT YAxis;                            // Y Axis
        SHORT ZAxis;                            // Z Axis
        SHORT XRotation;                        // X Rotation
        SHORT YRotation;                        // Y Rotation
        SHORT ZRotation;                        // Z Rotation
        SHORT Slider;                           // Slider
        SHORT Dial;                                     // Dial
        USHORT POV;                                     // POV
        UINT32 Buttons;                         // 32 Buttons
} JOYSTICK_STATE, * PJOYSTICK_STATE;

EXTERN_C BOOL __stdcall VJoy_Initialize(PCHAR name, PCHAR serial);
EXTERN_C VOID __stdcall VJoy_Shutdown();
EXTERN_C BOOL __stdcall VJoy_UpdateJoyState(int id, PJOYSTICK_STATE pJoyState);
