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
#include "opentrack/plugin-api.hpp"

#include "opentrack/win32-joystick.hpp"
#include "opentrack-compat/options.hpp"
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

class FTNoIR_Tracker : public ITracker
{
public:
    FTNoIR_Tracker();
    ~FTNoIR_Tracker();
    void start_tracker(QFrame *);
    void data(double *data);
    settings s;
    QString guid;
    static constexpr int AXIS_MAX = win32_joy_ctx::joy_axis_size - 1;
    win32_joy_ctx joy_ctx;
};

class TrackerControls: public ITrackerDialog
{
    Q_OBJECT
public:
    TrackerControls();
    void register_tracker(ITracker *) {}
    void unregister_tracker() {}
    Ui::UIJoystickControls ui;
    FTNoIR_Tracker* tracker;
    settings s;
    struct joys {
        QString name;
        QString guid;
    };
    QList<joys> _joys;
private slots:
    void doOK();
    void doCancel();
};

class FTNoIR_TrackerDll : public Metadata
{
public:
    QString name() { return QString("Joystick input"); }
    QIcon icon() { return QIcon(":/images/facetracknoir.png"); }
};

