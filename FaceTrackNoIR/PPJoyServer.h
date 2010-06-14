/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay for				*
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
* PPJoyServer		PPJoyServer is the Class, that communicates headpose-data	*
*					to the Virtual Joystick, created by Deon van der Westhuysen.*
********************************************************************************/
#pragma once
#ifndef INCLUDED_PPJOYSERVER_H
#define INCLUDED_PPJOYSERVER_H
 
#include "PPJIoctl.h"
#include <QString>
#include <QMessageBox>
#include <QSettings>
#include <QFile>
#include <QApplication>
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QLibrary>

#include "ui_FTNoIR_ppjoycontrols.h"

using namespace std;

class Tracker;				// pre-define parent-class to avoid circular includes

#define	NUM_ANALOG	7		/* Number of analog values which we will provide */
#define	NUM_DIGITAL	1		/* Number of digital values which we will provide */

#pragma pack(push,1)		/* All fields in structure must be byte aligned. */
typedef struct
{
 unsigned long	Signature;				/* Signature to identify packet to PPJoy IOCTL */
 char			NumAnalog;				/* Num of analog values we pass */
 long			Analog[NUM_ANALOG];		/* Analog values */
 char			NumDigital;				/* Num of digital values we pass */
 char			Digital[NUM_DIGITAL];	/* Digital values */
}	JOYSTICK_STATE;
#pragma pack(pop)

class PPJoyServer : public QThread {
	Q_OBJECT

public: 

	// public member methods
	PPJoyServer( Tracker *parent );
	~PPJoyServer();

	// protected member methods
protected:
	void run();

private slots:
//	void readPendingDatagrams();

private:
	// Handles to neatly terminate thread...
	HANDLE m_StopThread;
	HANDLE m_WaitThread;

	Tracker *headTracker;									// For upstream messages...
	
	HANDLE h;
	JOYSTICK_STATE JoyState;
	DWORD RetSize;
	DWORD rc;
	long *Analog;
	char *Digital;
	int selectedPPJoy;										// Number of virtual joystick (1..16)

//	static long analogDefault,PPJoyCorrection;
	long centerPos[3],centerRot[3];

	/** member variables for saving the head pose **/
	float virtPosX;
	float virtPosY;
	float virtPosZ;
	
	float virtRotX;
	float virtRotY;
	float virtRotZ;

	void checkAnalogLimits();
	long scale2AnalogLimits( float x, float min_x, float max_x );
	void loadSettings();

public:
	void setVirtRotX(float rot) { virtRotX = rot; }
	void setVirtRotY(float rot) { virtRotY = rot; }
	void setVirtRotZ(float rot) { virtRotZ = rot; }
	void setVirtPosX(float pos) { virtPosX = pos; }
	void setVirtPosY(float pos) { virtPosY = pos; }
	void setVirtPosZ(float pos) { virtPosZ = pos; }

};

// Widget that has controls for PPJoy server-settings.
class PPJoyControls: public QWidget, public Ui::UICPPJoyControls
{
    Q_OBJECT
public:

	explicit PPJoyControls( QWidget *parent=0, Qt::WindowFlags f=0 );
    virtual ~PPJoyControls();
	void showEvent ( QShowEvent * event );

private:
	Ui::UICPPJoyControls ui;
	void loadSettings();
	void save();

	/** helper **/
	bool settingsDirty;

private slots:
	void doOK();
	void doCancel();
	void virtualJoystickSelected( int index );

};


#endif//INCLUDED_PPJOYSERVER_H
//END
