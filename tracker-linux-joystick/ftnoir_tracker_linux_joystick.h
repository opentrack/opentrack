/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once
#include "ui_ftnoir_tracker_linux_joystick_controls.h"
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QSettings>
#include <QList>
#include <QFrame>
#include <QStringList>
#include <cmath>
#include "api/plugin-api.hpp"

#include <linux/joystick.h>
#include <fcntl.h>
#include <unistd.h>

#include "options/options.hpp"
using namespace options;

struct settings : opts {
    value<QString> guid;
    value<int> joy_1, joy_2, joy_3, joy_4, joy_5, joy_6;
    settings() :
        opts("tracker-linux-joystick"),
        guid(b, "joy-guid", ""),
        joy_1(b, "axis-map-1", 1),
        joy_2(b, "axis-map-2", 2),
        joy_3(b, "axis-map-3", 3),
        joy_4(b, "axis-map-4", 4),
        joy_5(b, "axis-map-5", 5),
        joy_6(b, "axis-map-6", 6)
    {}
};

struct linux_joystick {
    QString name;
    QString device_id;
    QString dev;
};
QList<linux_joystick> getJoysticks();
QString getJoystickDevice(QString guid);

class joystick : public ITracker
{
public:
    joystick();
    ~joystick();
    module_status start_tracker(QFrame *);
    void data(double *data);
    settings s;
    QString guid;
    static constexpr int AXIS_MAX = USHRT_MAX;
    static constexpr int JOY_CHANNELS = 8;
    int axes_state[JOY_CHANNELS] = {0};
    int joy_fd;
};

class dialog_joystick: public ITrackerDialog
{
    Q_OBJECT
public:
    dialog_joystick();
    void register_tracker(ITracker *) {}
    void unregister_tracker() {}
    Ui::UILinuxJoystickControls ui;
    joystick* tracker;
    settings s;
    struct joys {
        QString name;
        QString guid;
    };
    QList<joys> joys_;
private slots:
    void doOK();
    void doCancel();
};

class joystickDll : public Metadata
{
    Q_OBJECT

    QString name() { return tr("Linux Joystick input"); }
    QIcon icon() { return QIcon(":/images/opentrack.png"); }
};
