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
#include "facetracknoir/options.h"
using namespace options;

struct settings {
    pbundle b;
    value<double> fov, headpos_x, headpos_y, headpos_z;
    value<int> camera_index, force_fps, resolution;
    value<bool> red_only;
    value<bool> eyaw, epitch, eroll, ex, ey, ez;
    value<double> marker_pitch;
    settings() :
        b(bundle("aruco-tracker")),
        fov(b, "field-of-view", 56),
        headpos_x(b, "headpos-x", 0),
        headpos_y(b, "headpos-y", 0),
        headpos_z(b, "headpos-z", 0),
        camera_index(b, "camera-index", 0),
        force_fps(b, "force-fps", 0),
        resolution(b, "force-resolution", 0),
        red_only(b, "red-only", false),
        eyaw(b, "enable-y", true),
        epitch(b, "enable-p", true),
        eroll(b, "enable-r", true),
        ex(b, "enable-x", true),
        ey(b, "enable-y", true),
        ez(b, "enable-z", true),
        marker_pitch(b, "marker-pitch", 0)
    {}
};

class Tracker : protected QThread, public ITracker
{
    Q_OBJECT
public:
	Tracker();
    virtual ~Tracker();
    void StartTracker(QFrame* frame);
    void GetHeadPoseData(double *data);
    void run();
    void reload() { s.b->reload(); }
private:
    QMutex mtx;
    volatile bool stop;
    QHBoxLayout* layout;
	ArucoVideoWidget* videoWidget;
    settings s;
    double pose[6];
    cv::Mat frame;
    cv::VideoCapture camera;
};

// Widget that has controls for FTNoIR protocol client-settings.
class TrackerControls : public QWidget, public ITrackerDialog
{
    Q_OBJECT
public:
    TrackerControls();
    void registerTracker(ITracker * x) {
        tracker = dynamic_cast<Tracker*>(x);
    }
    void unRegisterTracker() {
        tracker = nullptr;
    }
private:
	Ui::Form ui;
    Tracker* tracker;
    settings s;
private slots:
	void doOK();
	void doCancel();
};

#endif

