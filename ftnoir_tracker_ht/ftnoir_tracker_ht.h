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
#include "facetracknoir/options.hpp"
using namespace options;

class Tracker : public QObject, public ITracker
{
    Q_OBJECT
public:
	Tracker();
    virtual ~Tracker();
    void StartTracker(QFrame* frame);
    void GetHeadPoseData(double *data);
    pbundle b;
    value<bool> enableTX, enableTY, enableTZ, enableRX, enableRY, enableRZ;
    value<double> fov;
    value<int> fps, camera_idx, resolution;
    void load_settings(ht_config_t* config);
private:
    PortableLockedShm lck_shm;
    ht_shm_t* shm;
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
    void Initialize(QWidget *);
    void registerTracker(ITracker *) {}
    void unRegisterTracker() {}

private:
	Ui::Form ui;
    pbundle b;
    value<bool> enableTX, enableTY, enableTZ, enableRX, enableRY, enableRZ;
    value<double> fov;
    value<int> fps, camera_idx, resolution;

private slots:
	void doOK();
	void doCancel();
};

#endif

