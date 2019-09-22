/* Copyright (c) 2015, 2019 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "ui_ftnoir_mousecontrols.h"

#include "mouse-settings.hpp"
#include "api/plugin-api.hpp"

#include <QDebug>

using namespace options;

class mouse : public TR, public IProtocol
{
    Q_OBJECT

    static int get_delta(int val, int prev);
    static int get_value(double val, double sensitivity, bool is_rotation);

    int last_x = 0, last_y = 0;
    mouse_settings s;

public:
    mouse() = default;
    module_status initialize() override { return status_ok(); }
    void pose(const double* headpose, const double*) override;
    QString game_name() override;
};

class MOUSEControls: public IProtocolDialog
{
    Q_OBJECT

    Ui::UI_mouse ui;
    mouse_settings s;

private slots:
    void doOK();
    void doCancel();

public:
    MOUSEControls();
    void register_protocol(IProtocol*) override {}
    void unregister_protocol() override {}
};

class mouseDll : public Metadata
{
    Q_OBJECT

    QString name() override { return tr("mouse emulation"); }
    QIcon icon() override { return QIcon(":/images/mouse.png"); }
};
