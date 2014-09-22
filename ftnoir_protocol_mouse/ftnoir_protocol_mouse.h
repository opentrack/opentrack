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

#include "ui_ftnoir_mousecontrols.h"
#include <QMessageBox>
#include <QSettings>
#include <QLibrary>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <windows.h>
#include <winuser.h>
#include "facetracknoir/plugin-api.hpp"
#include "facetracknoir/options.h"
using namespace options;

struct settings {
    pbundle b;
    value<int> Mouse_X, Mouse_Y;
    settings() :
        b(bundle("mouse-proto")),
        Mouse_X(b, "mouse-x", 0),
        Mouse_Y(b, "mouse-y", 0)
    {}
};

#define MOUSE_AXIS_MIN 0
#define MOUSE_AXIS_MAX 65535

class FTNoIR_Protocol : public IProtocol
{
public:
    FTNoIR_Protocol() {}
    bool checkServerInstallationOK();
    void sendHeadposeToGame( const double *headpose);
    QString getGameName() {
        return "Mouse tracker";
    }
    void reload();
private:
    struct settings s;
};

class MOUSEControls: public QWidget, public IProtocolDialog
{
    Q_OBJECT
public:
    MOUSEControls();
    void registerProtocol(IProtocol *protocol) {
        _proto = (FTNoIR_Protocol *) protocol;
    }
    void unRegisterProtocol() {
        _proto = NULL;
    }
private:
    Ui::UICMOUSEControls ui;
    settings s;
    FTNoIR_Protocol* _proto;
private slots:
    void doOK();
    void doCancel();
};

class FTNoIR_ProtocolDll : public Metadata
{
public:
    void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("Mouse Look"); }
    void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("Mouse Look"); }
    void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("Mouse Look protocol"); }
    void getIcon(QIcon *icon) { *icon = QIcon(":/images/mouse.png"); }
};


#endif//INCLUDED_MOUSESERVER_H
//END
