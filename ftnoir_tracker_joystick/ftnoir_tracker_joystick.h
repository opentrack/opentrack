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
#include <math.h>
#include "facetracknoir/global-settings.h"
#ifndef DIRECTINPUT_VERSION
#   define DIRECTINPUT_VERSION 0x800
#endif
#include <windows.h>
#include <commctrl.h>
#include <basetsd.h>
#include <dinput.h>
#include <dinputd.h>
#include <oleauto.h>
#include <shellapi.h>

#define AXIS_MAX 16383

struct DI_ENUM_CONTEXT
{
    DIDEVICEINSTANCE* pPreferredJoyCfg;
    GUID preferred_instance;
    LPDIRECTINPUTDEVICE8* g_pJoystick;
    LPDIRECTINPUT8 g_pDI;
};

class FTNoIR_Tracker : public ITracker
{
public:
	FTNoIR_Tracker();
	~FTNoIR_Tracker();

    void StartTracker(QFrame *frame);
    bool GiveHeadPoseData(double *data);
	void loadSettings();
    LPDIRECTINPUT8          g_pDI;
    LPDIRECTINPUTDEVICE8    g_pJoystick;
    int axes[6];
    GUID preferred;
    int joyid;
    QMutex mtx;
    QFrame* frame;
    DIDEVICEINSTANCE def;
    void reload();
    int iter; // XXX bad style
};

// Widget that has controls for FTNoIR protocol client-settings.
class TrackerControls: public QWidget, public ITrackerDialog
{
    Q_OBJECT
public:
	explicit TrackerControls();
    ~TrackerControls();
    void showEvent (QShowEvent *);

    void Initialize(QWidget *parent);
    void registerTracker(ITracker *foo) {
        tracker = dynamic_cast<FTNoIR_Tracker*>(foo);
    }
    void unRegisterTracker() {
        tracker = NULL;
    }
    QList<GUID> guids;
    Ui::UIJoystickControls ui;
    void loadSettings();
	void save();
    bool settingsDirty;
    FTNoIR_Tracker* tracker;

private slots:
	void doOK();
	void doCancel();
    void settingChanged() { settingsDirty = true; }
    void settingChanged(int) { settingsDirty = true; }
};

//*******************************************************************************************************
// FaceTrackNoIR Tracker DLL. Functions used to get general info on the Tracker
//*******************************************************************************************************
class FTNoIR_TrackerDll : public Metadata
{
public:
	FTNoIR_TrackerDll();
	~FTNoIR_TrackerDll();

	void getFullName(QString *strToBeFilled);
	void getShortName(QString *strToBeFilled);
	void getDescription(QString *strToBeFilled);
	void getIcon(QIcon *icon);

private:
	QString trackerFullName;									// Trackers' name and description
	QString trackerShortName;
	QString trackerDescription;
};

