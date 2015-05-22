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
#include <QMutex>
#include <QFrame>
#include <cmath>
#include "opentrack/plugin-api.hpp"
#ifndef DIRECTINPUT_VERSION
#   define DIRECTINPUT_VERSION 0x800
#endif
#include <windows.h>
#include <commctrl.h>
#include <basetsd.h>
#include <dinput.h>
#include <oleauto.h>
#include <shellapi.h>

#include "opentrack/options.hpp"
using namespace options;

struct settings : opts {
    value<QString> joyid;
    value<int> joy_1, joy_2, joy_3, joy_4, joy_5, joy_6;
    settings() :
        opts("tracker-joystick"),
        joyid(b, "joy-id", ""),
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
    void start_tracker(QFrame *frame);
    void data(double *data);
    void reload();
    LPDIRECTINPUT8          g_pDI;
    LPDIRECTINPUTDEVICE8    g_pJoystick;
    QMutex mtx;
    QFrame* frame;
    DIDEVICEINSTANCE def;
    int iter; // XXX bad style
    settings s;
    static constexpr int AXIS_MAX = 65535;
};

class TrackerControls: public ITrackerDialog
{
    Q_OBJECT
public:
    TrackerControls();
    void register_tracker(ITracker *) {}
    void unregister_tracker() {}
    QList<GUID> guids;
    Ui::UIJoystickControls ui;
    FTNoIR_Tracker* tracker;
    settings s;
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

