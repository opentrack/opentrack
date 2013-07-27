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
#include "video_widget.h"
#include <QObject>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QHBoxLayout>
#include <opencv2/opencv.hpp>

class Tracker : public QThread, public ITracker
{
    Q_OBJECT
public:
	Tracker();
	~Tracker();
    void StartTracker(QFrame* frame);
    bool GiveHeadPoseData(double *data);
	bool enableTX, enableTY, enableTZ, enableRX, enableRY, enableRZ;
    bool NeedsTimeToFinish() {
        return true;
    }
    void WaitForExit() {
        stop = true;
        wait();
    }
    void run();
private:
    QMutex mtx;
    QTimer timer;
	VideoWidget* videoWidget;
	QHBoxLayout* layout;
    volatile bool fresh, stop;
    float fov;
    int camera_index;
    float dc[5];
    int force_fps, force_width, force_height;
    void load_settings();
    double pose[6];
    int marker_size;
    cv::Mat frame;
private slots:
    void paint_widget();
};

// Widget that has controls for FTNoIR protocol client-settings.
class TrackerControls : public QWidget, public ITrackerDialog
{
    Q_OBJECT
public:

	explicit TrackerControls();
    virtual ~TrackerControls();
	void showEvent ( QShowEvent * event );

    void Initialize(QWidget *parent);
    void registerTracker(ITracker *tracker) {}
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

