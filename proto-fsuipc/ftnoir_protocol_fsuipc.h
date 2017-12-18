/* Homepage         http://facetracknoir.sourceforge.net/home/default.htm        *
 *                                                                               *
 * ISC License (ISC)                                                             *
 *                                                                               *
 * Copyright (c) 2015, Wim Vriend                                                *
 *                                                                               *
 * Permission to use, copy, modify, and/or distribute this software for any      *
 * purpose with or without fee is hereby granted, provided that the above        *
 * copyright notice and this permission notice appear in all copies.             *
 */
#pragma once
#ifndef INCLUDED_FSUIPCSERVER_H
#define INCLUDED_FSUIPCSERVER_H

#include <windows.h>
#include <stdlib.h>
#include "FSUIPC_User.h"
#include "api/plugin-api.hpp"
#include "ui_ftnoir_fsuipccontrols.h"
#include <QMessageBox>
#include <QSettings>
#include <QLibrary>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include "options/options.hpp"
using namespace options;

#define FSUIPC_FILENAME "C:\\Program Files\\Microsoft Games\\Flight Simulator 9\\Modules\\FSUIPC.dll"

struct settings : opts {
    value<QString> LocationOfDLL;
    settings() :
        opts("proto-fsuipc"),
        LocationOfDLL(b, "dll-location", FSUIPC_FILENAME)
    {}
};

#pragma pack(push,1) // All fields in structure must be unaligned
typedef struct
{
    int Control; // Control identifier
    int Value;   // Value of DOF
} TFSState;
#pragma pack(pop)

class fsuipc : public IProtocol
{
public:
    fsuipc();
    ~fsuipc() override;
    module_status initialize() override;
    void pose(const double* headpose);
    QString game_name() { return otr_tr("Microsoft Flight Simulator X"); }
private:
    QLibrary FSUIPCLib;
    double prevPosX, prevPosY, prevPosZ, prevRotX, prevRotY, prevRotZ;
    settings s;

    template<typename t>
    static int scale2AnalogLimits(t x, t min_x, t max_x );
};

class FSUIPCControls: public IProtocolDialog
{
    Q_OBJECT
public:
    FSUIPCControls();
    void register_protocol(IProtocol *) {}
    void unregister_protocol() {}
private:
    Ui::UICFSUIPCControls ui;
    settings s;
private slots:
    void doOK();
    void doCancel();
    void getLocationOfDLL();
};

class fsuipcDll : public Metadata
{
public:
    QString name() { return otr_tr("FSUIPC -- Microsoft FS2002/FS2004"); }
    QIcon icon() { return QIcon(":/images/fs9.png"); }
};


#endif//INCLUDED_FSUIPCSERVER_H
//END
