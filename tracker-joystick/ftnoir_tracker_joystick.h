/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once
#include "ui_ftnoir_tracker_joystick_controls.h"
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

#include "input/win32-joystick.hpp"
#include "options/options.hpp"
using namespace options;

struct settings : opts {
    value<QString> guid;
    value<int> joy_1, joy_2, joy_3, joy_4, joy_5, joy_6;
    settings() :
        opts("tracker-joystick"),
        guid(b, "joy-guid", ""),
        joy_1(b, "axis-map-1", 1),
        joy_2(b, "axis-map-2", 2),
        joy_3(b, "axis-map-3", 3),
        joy_4(b, "axis-map-4", 4),
        joy_5(b, "axis-map-5", 5),
        joy_6(b, "axis-map-6", 6)
    {}
};

class joystick : public ITracker
{
public:
    joystick();
    ~joystick();
    module_status start_tracker(QFrame *) { return status_ok(); }
    void data(double *data);
    settings s;
    QString guid;
    static constexpr int AXIS_MAX = win32_joy_ctx::joy_axis_size;
    win32_joy_ctx joy_ctx;
};

class dialog_joystick: public ITrackerDialog
{
    Q_OBJECT
public:
    dialog_joystick();
    void register_tracker(ITracker *) {}
    void unregister_tracker() {}
    Ui::UIJoystickControls ui;
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

    QString name() { return tr("Joystick input"); }
    QIcon icon() { return QIcon(":/images/opentrack.png"); }
};
