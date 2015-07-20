/* Copyright (c) 2013-2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "ui_aruco-trackercontrols.h"
#include "ar_video_widget.h"
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QHBoxLayout>
#include <QDialog>
#include <QTimer>
#include "opentrack/options.hpp"
#include "ftnoir_tracker_aruco/trans_calib.h"
#include "opentrack/plugin-api.hpp"
#include "opentrack/opencv-camera-dialog.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace options;

struct settings : opts {
    value<double> fov, headpos_x, headpos_y, headpos_z;
    value<QString> camera_name;
    value<int> force_fps, resolution;
    settings() :
        opts("aruco-tracker"),
        fov(b, "field-of-view", 56),
        headpos_x(b, "headpos-x", 0),
        headpos_y(b, "headpos-y", 0),
        headpos_z(b, "headpos-z", 0),
        camera_name(b, "camera-name", ""),
        force_fps(b, "force-fps", 0),
        resolution(b, "force-resolution", 0)
    {}
};

class TrackerControls;

class Tracker : protected QThread, public ITracker
{
    Q_OBJECT
    friend class TrackerControls;
    static constexpr double c_search_window = 2.65;
public:
    Tracker();
    ~Tracker() override;
    void start_tracker(QFrame* frame);
    void data(double *data);
    void run();
    void getRT(cv::Matx33d &r, cv::Vec3d &t);
private:
    cv::VideoCapture camera;
    QMutex camera_mtx;
    QMutex mtx;
    volatile bool stop;
    QHBoxLayout* layout;
    ArucoVideoWidget* videoWidget;
    settings s;
    double pose[6];
    cv::Mat frame;
    cv::Matx33d r;
    cv::Vec3d t;
};

class TrackerControls : public ITrackerDialog, protected camera_dialog<Tracker>
{
    Q_OBJECT
public:
    TrackerControls();
    void register_tracker(ITracker * x) { tracker = static_cast<Tracker*>(x); }
    void unregister_tracker() { tracker = nullptr; }
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
    void camera_settings();
};

class TrackerDll : public Metadata
{
    QString name() { return QString("aruco -- paper marker tracker"); }
    QIcon icon() { return QIcon(":/images/aruco.png"); }
};
