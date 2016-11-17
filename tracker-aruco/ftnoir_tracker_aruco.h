/* Copyright (c) 2013-2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "ui_aruco-trackercontrols.h"
#include "options/options.hpp"
#include "trans_calib.h"
#include "api/plugin-api.hpp"
#include "cv/video-widget.hpp"
#include "compat/timer.hpp"

#include "include/markerdetector.h"

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QHBoxLayout>
#include <QDialog>
#include <QTimer>

#include <cinttypes>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

using namespace options;

struct settings : opts {
    value<int> fov;
    value<double> headpos_x, headpos_y, headpos_z;
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

class aruco_dialog;

class aruco_tracker : protected QThread, public ITracker
{
    Q_OBJECT
    friend class aruco_dialog;
    static constexpr float c_search_window = 1.3f;
public:
    aruco_tracker();
    ~aruco_tracker() override;
    void start_tracker(QFrame* frame) override;
    void data(double *data) override;
    void run() override;
    void getRT(cv::Matx33d &r, cv::Vec3d &t);
private:
    bool detect_with_roi();
    bool detect_without_roi();
    bool open_camera();
    void set_intrinsics();
    void update_fps();
    void draw_ar(bool ok);
    void clamp_last_roi();
    void set_points();
    void draw_centroid();
    void set_last_roi();
    void set_rmat();
    void set_roi_from_projection();

    cv::VideoCapture camera;
    QMutex camera_mtx;
    QMutex mtx;
    QHBoxLayout* layout;
    cv_video_widget* videoWidget;
    settings s;
    double pose[6], fps;
    cv::Mat frame, grayscale, color;
    cv::Matx33d r;
    std::vector<cv::Point3f> obj_points;
    cv::Matx33d intrinsics;
    aruco::MarkerDetector detector;
    std::vector<aruco::Marker> markers;
    cv::Vec3d t;
    cv::Vec3d rvec, tvec;
    std::vector<cv::Point2f> roi_projection;
    std::vector<cv::Point2f> repr2;
    cv::Matx33d m_r, m_q, rmat;
    cv::Vec3d euler;
    std::vector<cv::Point3f> roi_points;
    cv::Rect last_roi;
    Timer fps_timer;

    volatile bool stop;

    static constexpr const double size_min = 0.05f;
    static constexpr const double size_max = 0.3f;

    static constexpr const double RC = .25;
};

class aruco_dialog : public ITrackerDialog
{
    Q_OBJECT
public:
    aruco_dialog();
    void register_tracker(ITracker * x) override { tracker = static_cast<aruco_tracker*>(x); }
    void unregister_tracker() override { tracker = nullptr; }
private:
    Ui::Form ui;
    aruco_tracker* tracker;
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
    void update_camera_settings_state(const QString& name);
};

class aruco_metadata : public Metadata
{
    QString name() { return QString("aruco -- paper marker tracker"); }
    QIcon icon() { return QIcon(":/images/aruco.png"); }
};
