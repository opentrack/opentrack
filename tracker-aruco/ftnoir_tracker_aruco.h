/* Copyright (c) 2013-2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "ui_aruco-trackercontrols.h"
#include "options/options.hpp"
#include "cv/translation-calibrator.hpp"
#include "api/plugin-api.hpp"
#include "cv/video-widget.hpp"
#include "compat/timer.hpp"
#include "video/camera.hpp"

#include "aruco/markerdetector.h"

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QHBoxLayout>
#include <QDialog>
#include <QTimer>

#include <memory>
#include <cinttypes>

#include <opencv2/core.hpp>

// value 0->1
//#define DEBUG_UNSHARP_MASKING .75

//canny thresholding
//#define USE_EXPERIMENTAL_CANNY

using namespace options;

enum aruco_fps
{
    fps_default = 0,
    fps_30      = 1,
    fps_60      = 2,
    fps_75      = 3,
    fps_125     = 4,
    fps_200     = 5,
    fps_50      = 6,
    fps_100     = 7,
    fps_120     = 8,
    fps_300     = 9,
    fps_250     = 10,
    fps_MAX     = 11,
};

struct settings : opts {
    value<double> headpos_x { b, "headpos-x", 0 },
                  headpos_y { b, "headpos-y", 0 },
                  headpos_z { b, "headpos-z", 0 };

    value<QString> camera_name { b, "camera-name", ""};
    value<int> resolution { b, "force-resolution", 0 };
    value<int> fov { b, "field-of-view", 56 };
    value<aruco_fps> force_fps { b, "force-fps", fps_default };
    value<bool> use_mjpeg { b, "use-mjpeg", false };

    settings();
};

class aruco_tracker : protected virtual QThread, public ITracker
{
    //Q_OBJECT
    static constexpr float c_search_window = 1.3f;
public:
    aruco_tracker();
    ~aruco_tracker() override;
    module_status start_tracker(QFrame* frame) override;
    void data(double *data) override;
    void run() override;

    void getRT(cv::Matx33d &r, cv::Vec3d &t);
    QMutex camera_mtx;
    std::unique_ptr<video::impl::camera> camera;

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
    void set_detector_params();
    void cycle_detection_params();

    QMutex mtx;
    std::unique_ptr<cv_video_widget> videoWidget;
    std::unique_ptr<QHBoxLayout> layout;
    settings s;
    double pose[6] {}, fps = 0;
    double no_detection_timeout = 0;
    cv::Matx33d r;
    cv::Matx33d intrinsics = cv::Matx33d::eye();
    cv::Vec3d t;
    cv::Vec3d rvec, tvec;
    cv::Matx33d m_r, m_q, rmat = cv::Matx33d::eye();
    cv::Vec3d euler;
    std::vector<cv::Point3f> roi_points {4};
    std::vector<cv::Point2f> roi_projection;
    std::vector<cv::Point2f> repr2;
    std::vector<cv::Point3f> obj_points {4};
    aruco::MarkerDetector detector;
    std::vector<aruco::Marker> markers;
    cv::Mat frame, grayscale, color;
    cv::Rect last_roi { 65535, 65535, 0, 0 };
    Timer fps_timer, last_detection_timer;
    unsigned adaptive_size_pos { 0 };
    bool use_otsu = false;

#if !defined USE_EXPERIMENTAL_CANNY
    static constexpr int adaptive_thres = 6;
#else
    static constexpr int adaptive_thres = 3;
#endif

    static constexpr double timeout = .35;
    static constexpr double timeout_backoff_c = .25;

    static constexpr float size_min = 0.05f;
    static constexpr float size_max = 0.5f;

    static constexpr double RC = .25;

#ifdef DEBUG_UNSHARP_MASKING
    static constexpr double gauss_kernel_size = 3;
    cv::Mat blurred;
#endif
};

class aruco_dialog : public ITrackerDialog
{
    Q_OBJECT
public:
    aruco_dialog();
    void register_tracker(ITracker * x) override { tracker = static_cast<aruco_tracker*>(x); }
    void unregister_tracker() override { tracker = nullptr; }
private:
    void make_fps_combobox();

    Ui::Form ui;
    aruco_tracker* tracker;
    settings s;
    TranslationCalibrator calibrator;
    QTimer calib_timer;
private Q_SLOTS:
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
    Q_OBJECT
    QString name() override { return QString("aruco -- paper marker tracker"); }
    QIcon icon() override { return QIcon(":/images/aruco.png"); }
};
