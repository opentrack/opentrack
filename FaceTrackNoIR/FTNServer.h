/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
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
* FTNServer			FTNServer is the Class, that communicates headpose-data		*
*					to FlightGear, using UDP.				         			*
*					It is based on the (Linux) example made by Melchior FRANZ.	*
********************************************************************************/
#pragma once
#ifndef INCLUDED_FTNSERVER_H
#define INCLUDED_FTNSERVER_H
 
#include "FTNoIR_cxx_protocolserver.h"
#include "..\ftnoir_tracker_base\ftnoir_tracker_base.h"
#include <QString>
#include <QMessageBox>
#include <QSettings>
#include <QFile>
#include <QApplication>
#include <QDebug>
#include <QMutex>
#include <QLibrary>
#include <QUdpSocket>

using namespace std;
using namespace v4friend::ftnoir;

#include "ui_FTNoIR_FTNServercontrols.h"

class Tracker;				// pre-define parent-class to avoid circular includes

class FTNServer : public ProtocolServerBase {
	Q_OBJECT

public: 

	// public member methods
	FTNServer( Tracker *parent );
	~FTNServer();

	// protected member methods
protected:
	bool checkServerInstallationOK( HANDLE handle );
	void sendHeadposeToGame();
	void setVirtPosX(float pos) { virtPosX = pos; }
	void setVirtPosY(float pos) { virtPosY = pos; }
	void setVirtPosZ(float pos) { virtPosZ = pos; }

private:
	Tracker *headTracker;									// For upstream messages...
	THeadPoseData TestData;
	QUdpSocket *inSocket;									// Receive from FligthGear
	QUdpSocket *outSocket;									// Send to FligthGear
	qint32 cmd;
	qint32 fg_cmd;											// Command from FlightGear
	QHostAddress destIP;									// Destination IP-address
	int destPort;											// Destination port-number
	void loadSettings();
};

// Widget that has controls for FTNoIR protocol server-settings.
class FTNServerControls: public QWidget, public Ui::UICFTNServerControls
{
    Q_OBJECT
public:

	explicit FTNServerControls( QWidget *parent=0, Qt::WindowFlags f=0 );
    virtual ~FTNServerControls();
	void showEvent ( QShowEvent * event );

private:
	Ui::UICFTNServerControls ui;
	void loadSettings();
	void save();

	/** helper **/
	bool settingsDirty;

private slots:
	void doOK();
	void doCancel();
	void settingChanged() { settingsDirty = true; };
};



#endif//INCLUDED_FTNSERVER_H
//END
