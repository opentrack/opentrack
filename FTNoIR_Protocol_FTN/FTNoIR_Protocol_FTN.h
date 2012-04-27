/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2010-2011	Wim Vriend (Developing)								*
*							Ron Hendriks (Researching and Testing)				*
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

#include "..\ftnoir_protocol_base\ftnoir_protocol_base.h"
#include "..\ftnoir_tracker_base\ftnoir_tracker_base.h"
#include "ui_FTNoIR_FTNcontrols.h"
#include <QThread>
#include <QUdpSocket>
#include <QMessageBox>
#include <QSettings>
#include "Windows.h"
#include "math.h"

class FTNoIR_Protocol_FTN : public IProtocol
{
public:
	FTNoIR_Protocol_FTN();
	~FTNoIR_Protocol_FTN();

	void Release();
    void Initialize();

	bool checkServerInstallationOK( HANDLE handle );
	void sendHeadposeToGame( T6DOF *headpose );
	void getNameFromGame( char *dest );						// Take care dest can handle up to 100 chars...

	void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("FaceTrackNoIR"); };
	void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("FTN Client"); };
	void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("FaceTrackNoIR Client protocol"); };

private:
	THeadPoseData TestData;
	long frame_counter;
	QUdpSocket *inSocket;									// Receive from FaceTrackNoIR
	QUdpSocket *outSocket;									// Send to FaceTrackNoIR
	qint32 cmd;
	qint32 fg_cmd;											// Command from FlightGear
	QHostAddress destIP;									// Destination IP-address
	int destPort;											// Destination port-number
	void loadSettings();

};

// Widget that has controls for FTNoIR protocol client-settings.
class FTNControls: public QWidget, Ui::UICFTNControls, public IProtocolDialog
{
    Q_OBJECT
public:

	explicit FTNControls();
    virtual ~FTNControls();
	void showEvent ( QShowEvent * event );

	void Release();											// Member functions which are accessible from outside the DLL
    void Initialize(QWidget *parent);

	void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("FaceTrackNoIR"); };
	void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("FTN Client"); };
	void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("FaceTrackNoIR Client protocol"); };
	void getIcon(QIcon *icon) { *icon = QIcon(":/images/FaceTrackNoIR.ico"); };

private:
	Ui::UICFTNControls ui;
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
