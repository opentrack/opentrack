/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
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
#include "facetracknoir/global-settings.h"
#ifndef DIRECTINPUT_VERSION
#   define DIRECTINPUT_VERSION 0x800
#endif
#include <windows.h>
#include <commctrl.h>
#include <basetsd.h>
#include <dinput.h>
#include <oleauto.h>
#include <shellapi.h>

#include "facetracknoir/options.h"
using namespace options;

struct settings {
    pbundle b;
    value<int> axis_0;
    value<int> axis_1;
    value<int> axis_2;
    value<int> axis_3;
    value<int> axis_4;
    value<int> axis_5;
    value<QString> joyid;
    settings() :
        b(bundle("tracker-joystick")),
        axis_0(b, "axis-0", 0),
        axis_1(b, "axis-1", 0),
        axis_2(b, "axis-2", 0),
        axis_3(b, "axis-3", 0),
        axis_4(b, "axis-4", 0),
        axis_5(b, "axis-5", 0),
        joyid(b, "joy-id", "")
    {}
};

class FTNoIR_Tracker : public ITracker
{
public:
	FTNoIR_Tracker();
	~FTNoIR_Tracker();
    void StartTracker(QFrame *frame);
    void GetHeadPoseData(double *data);
    void reload();
    LPDIRECTINPUT8          g_pDI;
    LPDIRECTINPUTDEVICE8    g_pJoystick;
    int min_[8], max_[8];
    QMutex mtx;
    QFrame* frame;
    DIDEVICEINSTANCE def;
    int iter; // XXX bad style
    settings s;
};

class TrackerControls: public QWidget, public ITrackerDialog
{
    Q_OBJECT
public:
    TrackerControls();
    void registerTracker(ITracker *foo) {
        tracker = dynamic_cast<FTNoIR_Tracker*>(foo);
    }
    void unRegisterTracker() {
        tracker = NULL;
    }
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
	void getFullName(QString *strToBeFilled);
	void getShortName(QString *strToBeFilled);
	void getDescription(QString *strToBeFilled);
	void getIcon(QIcon *icon);
private:
	QString trackerFullName;									// Trackers' name and description
	QString trackerShortName;
	QString trackerDescription;
};

