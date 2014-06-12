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
#include "ftnoir_protocol_base/ftnoir_protocol_base.h"
#include "ui_ftnoir_fgcontrols.h"
#include "fgtypes.h"
#include <QThread>
#include <QUdpSocket>
#include <QMessageBox>
#include "facetracknoir/global-settings.h"
#include "facetracknoir/options.h"
using namespace options;

struct settings {
    pbundle b;
    value<int> ip1, ip2, ip3, ip4;
    value<int> port;
    settings() :
        b(bundle("flightgear-proto")),
        ip1(b, "ip1", 192),
        ip2(b, "ip2", 168),
        ip3(b, "ip3", 0),
        ip4(b, "ip4", 2),
        port(b, "port", 5542)
    {}
};

class FTNoIR_Protocol : public IProtocol
{
public:
    bool checkServerInstallationOK();
    void sendHeadposeToGame(const double *headpose);
    QString getGameName() {
        return "FlightGear";
    }
    void reloadSettings();
private:
    settings s;
    TFlightGearData FlightData;
    QUdpSocket outSocket;
};

// Widget that has controls for FTNoIR protocol client-settings.
class FGControls: public QWidget, public IProtocolDialog
{
    Q_OBJECT
public:
    FGControls();
	void registerProtocol(IProtocol *protocol) {
		theProtocol = (FTNoIR_Protocol *) protocol;			// Accept the pointer to the Protocol
	}
	void unRegisterProtocol() {
		theProtocol = NULL;									// Reset the pointer
	}
private:
	Ui::UICFGControls ui;
	FTNoIR_Protocol *theProtocol;
    settings s;
private slots:
	void doOK();
	void doCancel();
};

class FTNoIR_ProtocolDll : public Metadata
{
public:
	void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("FlightGear"); }
	void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("FlightGear"); }
	void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("FlightGear UDP protocol"); }
    void getIcon(QIcon *icon) { *icon = QIcon(":/images/flightgear.png"); }
};
