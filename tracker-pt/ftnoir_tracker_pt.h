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
#include <QTimer>

class TrackerDialog_PT;

namespace pt_module {

using namespace types;

class Tracker_PT : public QThread, public ITracker
{
    Q_OBJECT

    friend class ::TrackerDialog_PT;

    template<typename t>
    using pointer = typename pt_runtime_traits::pointer<t>;

public:
    Tracker_PT(pointer<pt_runtime_traits> pt_runtime_traits);
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
    pointer<pt_runtime_traits> traits;

    QMutex camera_mtx;
    QMutex data_mtx;

    PointTracker point_tracker;

    pt_settings s;

    std::unique_ptr<QLayout> layout;
    std::vector<vec2> points;

    int preview_width = 320, preview_height = 240;

    pointer<pt_point_extractor> point_extractor;
    pointer<pt_camera> camera;
    pointer<cv_video_widget> video_widget;
    pointer<pt_frame> frame;
    pointer<pt_preview> preview_frame;

    std::atomic<unsigned> point_count = 0;
    std::atomic<bool> ever_success = false;

    static constexpr f rad2deg = f(180/M_PI);
    //static constexpr float deg2rad = float(M_PI/180);
};

} // ns pt_impl

using pt_module::Tracker_PT;
