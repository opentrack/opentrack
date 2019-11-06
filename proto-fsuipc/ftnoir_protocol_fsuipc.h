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

#if 0
#define FSUIPC_FILENAME "C:\\Program Files\\Microsoft Games\\Flight Simulator 9\\Modules\\FSUIPC.dll"

struct settings : opts {
    value<QString> LocationOfDLL;
    settings() :
        opts("proto-fsuipc"),
        LocationOfDLL(b, "dll-location", FSUIPC_FILENAME)
    {}
};
#endif

#pragma pack(push,1) // All fields in structure must be unaligned
struct state
{
    int Control; // Control identifier
    int Value;   // Value of DOF
};
#pragma pack(pop)

class fsuipc : public TR, public IProtocol
{
    Q_OBJECT

public:
    fsuipc();
    ~fsuipc() override;
    module_status initialize() override;
    void pose(const double* headpose, const double*) override;
    QString game_name() override { return tr("Microsoft Flight Simulator X"); }
private:
#if 0
    QLibrary FSUIPCLib;

    double prevPosX = 0, prevPosY = 0, prevPosZ = 0,
           prevRotX = 0, prevRotY = 0, prevRotZ = 0;
    settings s;
#endif

    template<typename t>
    static int scale(t x, t min_x, t max_x);
};

class FSUIPCControls: public IProtocolDialog
{
    Q_OBJECT
public:
    FSUIPCControls();
    void register_protocol(IProtocol *) override {}
    void unregister_protocol() override {}
private:
    Ui::UICFSUIPCControls ui;
#if 0
    settings s;
#endif
private slots:
    void doCancel();
#if 0
    void doOK();
    void getLocationOfDLL();
#endif
};

class fsuipcDll : public Metadata
{
    Q_OBJECT

    QString name() override { return tr("FSUIPC -- Microsoft FS2002/FS2004"); }
    QIcon icon() override { return QIcon(":/images/fs9.png"); }
};


#endif//INCLUDED_FSUIPCSERVER_H
//END
