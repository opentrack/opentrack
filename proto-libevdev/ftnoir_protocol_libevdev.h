/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once
#include "ui_ftnoir_libevdev_controls.h"

#include "api/plugin-api.hpp"
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

#include <QMessageBox>

class evdev : public TR, public IProtocol
{
    Q_OBJECT

public:
    evdev();
    ~evdev() override;
    void pose(const double *headpose, const double*) override;
    QString game_name() override { return tr("Virtual joystick for Linux"); }
    module_status initialize() override;

private:
    struct libevdev* dev = nullptr;
    struct libevdev_uinput* uidev = nullptr;
    int error_code = 0;
    const char* error_expr = nullptr;
};

class LibevdevControls: public IProtocolDialog
{
    Q_OBJECT

public:
    LibevdevControls();
    void register_protocol(IProtocol *) {}
    void unregister_protocol() {}

private:
    Ui::UICLibevdevControls ui;
    void save();

private slots:
    void doOK();
    void doCancel();
};

class evdevDll : public Metadata
{
    Q_OBJECT

    QString name() override { return tr("libevdev joystick receiver"); }
    QIcon icon() override { return QIcon(":/images/linux.png"); }
};
