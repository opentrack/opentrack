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
#include "opentrack/plugin-api.hpp"
#include "opentrack/options.hpp"
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
    FTNoIR_Protocol() : last_x(0), last_y(0) {}
    bool correct();
    void pose( const double *headpose);
    QString game_name() {
        return "Mouse tracker";
    }
    int last_x, last_y;
private:
    struct settings s;
};

class MOUSEControls: public IProtocolDialog
{
    Q_OBJECT
public:
    MOUSEControls();
    void register_protocol(IProtocol *) {}
    void unregister_protocol() {}
private:
    Ui::UICMOUSEControls ui;
    settings s;
private slots:
    void doOK();
    void doCancel();
};

class FTNoIR_ProtocolDll : public Metadata
{
public:
    QString name() { return QString("mouse emulation"); }
    QIcon icon() { return QIcon(":/images/mouse.png"); }
};


#endif//INCLUDED_MOUSESERVER_H
//END
