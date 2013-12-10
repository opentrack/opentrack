/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef FTNOIR_TRACKER_HT_H
#define FTNOIR_TRACKER_HT_H

#include "stdafx.h"
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "headtracker-ftnoir.h"
#include "ui_ht-trackercontrols.h"
#include "ht_video_widget.h"
#include "compat/compat.h"
#include <QObject>

class Tracker : public QObject, public ITracker
{
    Q_OBJECT
public:
	Tracker();
    virtual ~Tracker();
    void StartTracker(QFrame* frame);
    void GetHeadPoseData(double *data);
	bool enableTX, enableTY, enableTZ, enableRX, enableRY, enableRZ;
	ht_shm_t* shm;
private:
    PortableLockedShm lck_shm;
	QProcess subprocess;
    HTVideoWidget* videoWidget;
	QHBoxLayout* layout;
};

// Widget that has controls for FTNoIR protocol client-settings.
class TrackerControls : public QWidget, public ITrackerDialog
{
    Q_OBJECT
public:

	explicit TrackerControls();
    virtual ~TrackerControls();
    void showEvent (QShowEvent *);

    void Initialize(QWidget *);
    void registerTracker(ITracker *) {}
    void unRegisterTracker() {}

private:
	Ui::Form ui;
	void loadSettings();
	void save();
	bool settingsDirty;

private slots:
	void doOK();
	void doCancel();
    void settingChanged() { settingsDirty = true; }
    void settingChanged(int) { settingsDirty = true; }
    void settingChanged(double) { settingsDirty = true; }
};

#endif

