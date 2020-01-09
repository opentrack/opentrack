/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once
#include "ui_edtracker_controls.h"
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QSettings>
#include <QList>
#include <QFrame>
#include <QStringList>
#include <QSerialPort>
#include <QSerialPortInfo>

#include <cmath>
#include "api/plugin-api.hpp"

#include "dinput/win32-joystick.hpp"
#include "options/options.hpp"
using namespace options;

struct settings : opts {
    value<QString> guid;
    value<int> joy_1, joy_2, joy_3, joy_4, joy_5, joy_6;
    value<bool> joy_1_abs, joy_2_abs, joy_3_abs, joy_4_abs, joy_5_abs, joy_6_abs;
    value<QString> com_port_name;
    settings() :
        opts("tracker-edtracker"),
        guid(b, "joy-guid", ""),
        joy_1(b, "axis-map-1", 1),
        joy_2(b, "axis-map-2", 2),
        joy_3(b, "axis-map-3", 3),
        joy_4(b, "axis-map-4", 4),
        joy_5(b, "axis-map-5", 5),
        joy_6(b, "axis-map-6", 6),
        joy_1_abs(b, "axis-map-1-abs", false),
        joy_2_abs(b, "axis-map-2-abs", false),
        joy_3_abs(b, "axis-map-3-abs", false),
        joy_4_abs(b, "axis-map-4-abs", false),
        joy_5_abs(b, "axis-map-5-abs", false),
        joy_6_abs(b, "axis-map-6-abs", false),
        com_port_name(b, "com-port-name", "")
    {}
};

class edtracker : public ITracker
{
public:
    edtracker();
    ~edtracker();
    module_status start_tracker(QFrame*) { calibrate(); return status_ok(); }
    bool center() { reset(); return true; }
    void data(double *data);
    settings s;
    QString guid;
    QString com_port_name;
    static constexpr int AXIS_MAX = win32_joy_ctx::joy_axis_size;
    win32_joy_ctx joy_ctx;
    void reset();
    void calibrate();
};

class dialog_edtracker: public ITrackerDialog
{
    Q_OBJECT
public:
    dialog_edtracker();
    void register_tracker(ITracker *) {}
    void unregister_tracker() {}
    Ui::UIEDTrackerControls ui;
    edtracker* tracker;
    settings s;
    struct joys {
        QString name;
        QString guid;
    };
    QList<joys> joys_;
    struct ports {
        QString port;
        QString serial;
    };
    QList<ports> ports_;
private slots:
    void doOK();
    void doCancel();
};

class edtrackerDll : public Metadata
{
    Q_OBJECT

    QString name() { return tr("EDTracker input"); }
    QIcon icon() { return QIcon(":/images/opentrack.png"); }
};
