/* Copyright (c) 2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "ui_ftnoir_mousecontrols.h"

#include "mouse-settings.hpp"
#include "compat/tr.hpp"

#include <QDebug>
#include "api/plugin-api.hpp"
using namespace options;

class mouse : public TR, public IProtocol
{
    Q_OBJECT

public:
    mouse() = default;
    module_status initialize() override { return status_ok(); }
    void pose(const double* headpose) override;
    QString game_name() override;

    int last_x = 0, last_y = 0;
private:
    static int get_delta(int val, int prev);
    static int get_value(double val, double sensitivity, bool is_rotation);

    struct mouse_settings s;
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
    mouse_settings s;
private slots:
    void doOK();
    void doCancel();
};

class mouseDll : public Metadata
{
    Q_OBJECT

    QString name() { return tr("mouse emulation"); }
    QIcon icon() { return QIcon(":/images/mouse.png"); }
};
