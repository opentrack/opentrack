/* Copyright (c) 2013-2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "ui_aruco-trackercontrols.h"
#include "ar_video_widget.h"
#include "opentrack-compat/options.hpp"
#include "trans_calib.h"
#include "opentrack/plugin-api.hpp"
#include "opentrack/opencv-camera-dialog.hpp"
#include "include/markerdetector.h"

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QHBoxLayout>
#include <QDialog>
#include <QTimer>

#include <cinttypes>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

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

class TrackerControls;

class Tracker : protected QThread, public ITracker
{
    Q_OBJECT
    friend class TrackerControls;
    static constexpr float c_search_window = 1.3f;
public:
    Tracker();
    ~Tracker() override;
    void start_tracker(QFrame* frame) override;
    void data(double *data) override;
    void run() override;
    void getRT(cv::Matx33d &r, cv::Vec3d &t);
private:
    bool detect_with_roi();
    bool detect_without_roi();
    bool open_camera();
    void set_intrinsics();
    void update_fps(double dt);
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
    volatile bool stop;
    QHBoxLayout* layout;
    ArucoVideoWidget* videoWidget;
    settings s;
    double pose[6];
    cv::Mat frame, grayscale, color;
    cv::Matx33d r;
    std::vector<cv::Point3f> obj_points;
    cv::Matx33d intrinsics;
    cv::Matx14f dist_coeffs;
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
    double freq, cur_fps;
    std::uint64_t last_time;

    static constexpr float size_min = 0.05f;
    static constexpr float size_max = 0.3f;

    static constexpr double alpha_ = .95;
};

class TrackerControls : public ITrackerDialog, protected camera_dialog
{
    Q_OBJECT
public:
    TrackerControls();
    void register_tracker(ITracker * x) override { tracker = static_cast<Tracker*>(x); }
    void unregister_tracker() override { tracker = nullptr; }
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
