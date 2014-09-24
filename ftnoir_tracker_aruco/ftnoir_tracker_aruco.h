/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef FTNOIR_TRACKER_HT_H
#define FTNOIR_TRACKER_HT_H

#include "ui_aruco-trackercontrols.h"
#include "ar_video_widget.h"
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QHBoxLayout>
#include <QDialog>
#include <QTimer>
#include <opencv2/opencv.hpp>
#include <opencv/highgui.h>
#include "facetracknoir/options.h"
#include "ftnoir_tracker_aruco/trans_calib.h"
#include "facetracknoir/plugin-api.hpp"

#include "facetracknoir/gain-control.hpp"

using namespace options;

struct settings {
    pbundle b;
    value<double> fov, headpos_x, headpos_y, headpos_z;
    value<int> camera_index, force_fps, resolution;
    value<bool> red_only;
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
        marker_pitch(b, "marker-pitch", 0)
    {}
};

class Tracker : protected QThread, public ITracker
{
    Q_OBJECT
public:
	Tracker();
    ~Tracker() override;
    void StartTracker(QFrame* frame);
    void GetHeadPoseData(double *data);
    void run();
    void reload() { s.b->reload(); }
    void getRT(cv::Matx33f& r, cv::Vec3f& t);
private:
    QMutex mtx;
    volatile bool stop;
    QHBoxLayout* layout;
	ArucoVideoWidget* videoWidget;
    settings s;
    double pose[6];
    cv::Mat frame;
    cv::VideoCapture camera;
    cv::Matx33f r;
    cv::Vec3f t;
    Gain gain;
};

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
    TranslationCalibrator calibrator;
    QTimer calib_timer;
private slots:
	void doOK();
	void doCancel();
    void toggleCalibrate();
    void cleanupCalib();
    void update_tracker_calibration();
};

#endif

