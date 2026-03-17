/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once

#include <QThread>
#include <QHBoxLayout>
#include <QMutex>
#include "api/plugin-api.hpp"
#include "cv/video-widget.hpp"
#include "video/camera.hpp"
#include "compat/timer.hpp"
#include "aruco/markerdetector.h"
#include "arucohead-dialog.h"
#include "head.h"

class arucohead_dialog;

class arucohead_tracker : protected virtual QThread, public ITracker
{
public:
    arucohead_tracker();
    ~arucohead_tracker() override;
    module_status start_tracker(QFrame *) override;
    void data(double *data) override;
    void run() override;

private:
    arucohead::Head head;
    aruco::MarkerDetector detector;
    std::unique_ptr<video::impl::camera> camera;
    cv::Mat camera_matrix;
    std::vector<double> dist_coeffs;
    bool has_marker;
    settings s;
    std::unique_ptr<cv_video_widget> videoWidget;
    std::unique_ptr<QHBoxLayout> layout;
    Timer fps_timer;
    double fps = 0;
    QMutex camera_mtx;
    QMutex data_mtx;

    bool open_camera();
    void process_frame(cv::Mat& frame);
    cv::Mat build_camera_matrix(int image_width, int image_height, double diagonal_fov);
    void draw_head_bounding_box(cv::Mat &image);
    void draw_marker_border(cv::Mat &image, const std::vector<cv::Point2f> &image_points, int id);
    void draw_axes(cv::Mat &image, const cv::Vec3d &rvec, const cv::Vec3d &tvec, double axis_length=1, bool color=true);
    void update_fps();

    friend class arucohead_dialog;
};

class arucohead_metadata : public Metadata
{
    Q_OBJECT

    QString name() override { return tr("ArUcoHead Tracker"); }
    QIcon icon() override { return QIcon(":/images/opentrack.png"); }
};

