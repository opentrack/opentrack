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
#ifndef INCLUDED_FGSERVER_H
#define INCLUDED_FGSERVER_H

#include "ftnoir_protocol_base/ftnoir_protocol_base.h"
#include "ui_ftnoir_fgcontrols.h"
#include "fgtypes.h"
#include <QThread>
#include <QUdpSocket>
#include <QMessageBox>
#include <QSettings>
#include "facetracknoir/global-settings.h"

#define FT_PROGRAMID "FT_ProgramID"

class FTNoIR_Protocol : public IProtocol
{
public:
	FTNoIR_Protocol();
    virtual ~FTNoIR_Protocol();
    bool checkServerInstallationOK();
    void sendHeadposeToGame(const double *headpose);
    QString getGameName() {
        return "FlightGear";
    }
private:
    TFlightGearData FlightData;
    QUdpSocket outSocket;									// Send to FligthGear
	QHostAddress destIP;									// Destination IP-address
	int destPort;											// Destination port-number
	void loadSettings();
};

// Widget that has controls for FTNoIR protocol client-settings.
class FGControls: public QWidget, public IProtocolDialog
{
    Q_OBJECT
public:

	explicit FGControls();
    virtual ~FGControls();
	void showEvent ( QShowEvent * event );

    void Initialize(QWidget *parent);
	void registerProtocol(IProtocol *protocol) {
		theProtocol = (FTNoIR_Protocol *) protocol;			// Accept the pointer to the Protocol
	}
	void unRegisterProtocol() {
		theProtocol = NULL;									// Reset the pointer
	}

private:
	Ui::UICFGControls ui;
	void loadSettings();
	void save();

	/** helper **/
	bool settingsDirty;
	FTNoIR_Protocol *theProtocol;

private slots:
	void doOK();
	void doCancel();
	void chkLocalPCOnlyChanged();
	void settingChanged() { settingsDirty = true; }
};

//*******************************************************************************************************
// FaceTrackNoIR Protocol DLL. Functions used to get general info on the Protocol
//*******************************************************************************************************
class FTNoIR_ProtocolDll : public Metadata
{
public:
	FTNoIR_ProtocolDll();
	~FTNoIR_ProtocolDll();

	void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("FlightGear"); }
	void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("FlightGear"); }
	void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("FlightGear UDP protocol"); }

    void getIcon(QIcon *icon) { *icon = QIcon(":/images/flightgear.png"); }
};


#endif//INCLUDED_FGSERVER_H
//END
