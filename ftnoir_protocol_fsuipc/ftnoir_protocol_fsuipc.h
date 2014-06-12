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
* FSUIPCServer		FSUIPCServer is the Class, that communicates headpose-data	*
*					to games, using the FSUIPC.dll.			         			*
********************************************************************************/
#pragma once
#ifndef INCLUDED_FSUIPCSERVER_H
#define INCLUDED_FSUIPCSERVER_H

#include <windows.h>
#include <stdlib.h>
#include "FSUIPC_User.h"
#include "facetracknoir/global-settings.h"
#include "ftnoir_protocol_base/ftnoir_protocol_base.h"
#include "ui_ftnoir_fsuipccontrols.h"
#include <QMessageBox>
#include <QSettings>
#include <QLibrary>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include "facetracknoir/options.h"
using namespace options;

#define FSUIPC_FILENAME "C:\\Program Files\\Microsoft Games\\Flight Simulator 9\\Modules\\FSUIPC.dll"

struct settings {
    pbundle b;
    value<QString> LocationOfDLL;
    settings() :
        b(bundle("proto-fsuipc")),
        LocationOfDLL(b, "dll-location", FSUIPC_FILENAME)
    {}
};

#pragma pack(push,1)		// All fields in structure must be byte aligned.
typedef struct
{
 int Control;				// Control identifier
 int Value;					// Value of DOF
} TFSState;
#pragma pack(pop)

class FTNoIR_Protocol : public IProtocol
{
public:
	FTNoIR_Protocol();
    virtual ~FTNoIR_Protocol() virt_override;
    bool checkServerInstallationOK();
    void sendHeadposeToGame(const double* headpose);
    QString getGameName() {
        return "Microsoft Flight Simulator X";
    }
private:
	QLibrary FSUIPCLib;
    double prevPosX, prevPosY, prevPosZ, prevRotX, prevRotY, prevRotZ;
	static int scale2AnalogLimits( float x, float min_x, float max_x );
    settings s;
};

class FSUIPCControls: public QWidget, public IProtocolDialog
{
    Q_OBJECT
public:
    FSUIPCControls();
    void registerProtocol(IProtocol *) {}
    void unRegisterProtocol() {}
private:
	Ui::UICFSUIPCControls ui;
    settings s;
private slots:
	void doOK();
	void doCancel();
	void getLocationOfDLL();
};

class FTNoIR_ProtocolDll : public Metadata
{
public:
    void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("FS2002/FS2004"); }
    void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("FSUIPC"); }
    void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("Microsoft FS2004 protocol"); }
    void getIcon(QIcon *icon) { *icon = QIcon(":/images/fs9.png"); }
};


#endif//INCLUDED_FSUIPCSERVER_H
//END
