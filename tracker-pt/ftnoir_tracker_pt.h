/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014-2016 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "api/plugin-api.hpp"

#include "cv/numeric.hpp"

#include "pt-api.hpp"
#include "point_tracker.h"
#include "cv/video-widget.hpp"
#include "compat/util.hpp"

#include <atomic>
#include <memory>
#include <vector>

#include <opencv2/core.hpp>

#include <QThread>
#include <QMutex>
#include <QLayout>

class TrackerDialog_PT;

namespace pt_impl {

using namespace types;

class Tracker_PT : public QThread, public ITracker
{
    Q_OBJECT

    friend class ::TrackerDialog_PT;

public:
    Tracker_PT(const pt_runtime_traits& pt_runtime_traits);
    ~Tracker_PT() override;
    module_status start_tracker(QFrame* parent_window) override;
    void data(double* data) override;
    bool center() override;

    Affine pose();
    int  get_n_points();
    bool get_cam_info(pt_camera_info* info);
public slots:
    void maybe_reopen_camera();
    void set_fov(int value);
protected:
    void run() override;
private:
    QMutex camera_mtx;
    QMutex data_mtx;

    std::unique_ptr<pt_point_extractor> point_extractor;
    std::unique_ptr<pt_camera> camera;
    PointTracker point_tracker;

    pt_settings s;

    std::unique_ptr<cv_video_widget> video_widget;
    std::unique_ptr<QLayout> layout;

    cv::Mat frame, preview_frame;
    std::vector<vec2> points;

    QSize preview_size;

    std::atomic<unsigned> point_count = 0;
    std::atomic<bool> ever_success = false;

    static constexpr f rad2deg = f(180/M_PI);
    //static constexpr float deg2rad = float(M_PI/180);
};

} // ns pt_impl

using pt_impl::Tracker_PT;
