/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef FTNOIR_TRACKER_HT_H
#define FTNOIR_TRACKER_HT_H

#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "ui_aruco-trackercontrols.h"
#include "ar_video_widget.h"
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QHBoxLayout>
#include <QDialog>
#include <opencv2/opencv.hpp>
#include <opencv/highgui.h>

class Tracker : protected QThread, public ITracker
{
    Q_OBJECT
public:
	Tracker();
    virtual ~Tracker();
    void StartTracker(QFrame* frame);
    void GetHeadPoseData(double *data);
	bool enableTX, enableTY, enableTZ, enableRX, enableRY, enableRZ;
    void run();
    void load_settings();
private:
    QMutex mtx;
	ArucoVideoWidget* videoWidget;
	QHBoxLayout* layout;
    volatile bool stop;
    float fov;
    int camera_index;
    int force_fps, force_width, force_height;
    double pose[6];
    cv::Mat frame;
    double headpos[3], headpitch;
    cv::VideoCapture camera;
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
    void registerTracker(ITracker * x) {
        tracker = dynamic_cast<Tracker*>(x);
    }
    void unRegisterTracker() {
        tracker = nullptr;
    }

private:
	Ui::Form ui;
	void loadSettings();
	void save();
	bool settingsDirty;
    Tracker* tracker;

private slots:
	void doOK();
	void doCancel();
    void settingChanged() { settingsDirty = true; }
    void settingChanged(int) { settingsDirty = true; }
    void settingChanged(double) { settingsDirty = true; }
};

#endif

