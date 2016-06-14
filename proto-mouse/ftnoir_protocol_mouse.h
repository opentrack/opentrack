/* Copyright (c) 2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "ui_ftnoir_mousecontrols.h"
#include <QDebug>
#include "opentrack/plugin-api.hpp"
#include "opentrack-compat/options.hpp"
using namespace options;

struct settings : opts {
    value<int> Mouse_X, Mouse_Y;
    value<slider_value> sensitivity_x, sensitivity_y;
    settings() :
        opts("mouse-proto"),
        Mouse_X(b, "mouse-x", 0),
        Mouse_Y(b, "mouse-y", 0),
        sensitivity_x(b, "mouse-sensitivity-x", slider_value(200, 100, 500)),
        sensitivity_y(b, "mouse-sensitivity-y", slider_value(200, 100, 500))
    {}
};

class FTNoIR_Protocol : public IProtocol
{
public:
    FTNoIR_Protocol() : last_pos_x(0), last_pos_y(0), last_x(0), last_y(0) {}
    bool correct() override;
    void pose( const double *headpose) override;
    QString game_name() override;

    double last_pos_x, last_pos_y;
    int last_x, last_y;
private:
    static double get_rotation(double val, double last_val);
    static int get_value(double val, double& last_pos, int& last_px, bool is_rotation, double sensitivity);

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
