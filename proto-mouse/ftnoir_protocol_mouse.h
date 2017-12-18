/* Copyright (c) 2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "ui_ftnoir_mousecontrols.h"
#include <QDebug>
#include "api/plugin-api.hpp"
#include "options/options.hpp"
using namespace options;

struct settings : opts {
    value<int> Mouse_X, Mouse_Y;
    value<slider_value> sensitivity_x, sensitivity_y;
    settings() :
        opts("mouse-proto"),
        Mouse_X(b, "mouse-x", 0),
        Mouse_Y(b, "mouse-y", 0),
        sensitivity_x(b, "mouse-sensitivity-x", slider_value(200, 25, 500)),
        sensitivity_y(b, "mouse-sensitivity-y", slider_value(200, 25, 500))
    {}
};

class mouse : public IProtocol
{
public:
    mouse();
    module_status initialize() override { return status_ok(); }
    void pose( const double *headpose) override;
    QString game_name() override;

    int last_x, last_y;
private:
    static int get_delta(int val, int prev);
    static int get_value(double val, double sensitivity, bool is_rotation);

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

class mouseDll : public Metadata
{
public:
    QString name() { return otr_tr("mouse emulation"); }
    QIcon icon() { return QIcon(":/images/mouse.png"); }
};
