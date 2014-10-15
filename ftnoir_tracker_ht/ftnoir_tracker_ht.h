/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef FTNOIR_TRACKER_HT_H
#define FTNOIR_TRACKER_HT_H

#include "stdafx.h"
#include "headtracker-ftnoir.h"
#include "ui_ht-trackercontrols.h"
#include "ht_video_widget.h"
#include "compat/compat.h"
#include <QObject>
#include "facetracknoir/options.h"
#include "facetracknoir/plugin-api.hpp"
using namespace options;

struct settings {
    pbundle b;
    value<double> fov;
    value<int> fps, camera_idx, resolution;
    settings() :
        b(bundle("HT-Tracker")),
        fov(b, "fov", 56),
        fps(b, "fps", 0),
        camera_idx(b, "camera-index", 0),
        resolution(b, "resolution", 0)
    {}
};

class Tracker : public QObject, public ITracker
{
    Q_OBJECT
public:
	Tracker();
    ~Tracker() override;
    void StartTracker(QFrame* frame);
    void GetHeadPoseData(double *data);
    void load_settings(ht_config_t* config);
private:
    settings s;
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
    void registerTracker(ITracker *) {}
    void unRegisterTracker() {}

private:
	Ui::Form ui;
    settings s;

private slots:
	void doOK();
	void doCancel();
};

#endif

