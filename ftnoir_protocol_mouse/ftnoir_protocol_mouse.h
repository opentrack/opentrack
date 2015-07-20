/* Copyright (c) 2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "ui_ftnoir_mousecontrols.h"
#include <QDebug>
#include <windows.h>
#include "opentrack/plugin-api.hpp"
#include "opentrack/options.hpp"
using namespace options;

struct settings : opts {
    value<int> Mouse_X, Mouse_Y;
    settings() :
        opts("mouse-proto"),
        Mouse_X(b, "mouse-x", 0),
        Mouse_Y(b, "mouse-y", 0)
    {}
};

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
