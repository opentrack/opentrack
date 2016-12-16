/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#undef NDEBUG
#include <cassert>

#include "numeric.hpp"
#include "ftnoir_tracker_pt_settings.h"

#include "compat/util.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/videoio.hpp>

#include <memory>
#include <QString>

namespace impl {

using namespace types;

struct CamInfo final
{
    CamInfo() : fov(0), res_x(0), res_y(0), fps(-1), idx(-1) {}
    void get_focal_length(f& fx) const;

    double fov;

    int res_x;
    int res_y;
    int fps;
    int idx;
};

class Camera final
{
public:
    Camera() : dt_valid(0), dt_mean(0) {}

    DEFUN_WARN_UNUSED bool start(int idx, int fps, int res_x, int res_y);
    void stop();

    DEFUN_WARN_UNUSED bool get_frame(double dt, cv::Mat& frame, CamInfo& info);
    DEFUN_WARN_UNUSED bool get_info(CamInfo &ret) const;

    CamInfo get_desired() const { return cam_desired; }
    QString get_desired_name() const;
    QString get_active_name() const;

    cv::VideoCapture& operator*() { assert(cap); return *cap; }
    const cv::VideoCapture& operator*() const { assert(cap); return *cap; }
    cv::VideoCapture* operator->() { assert(cap); return cap.get(); }
    const cv::VideoCapture* operator->() const { return cap.get(); }
    operator bool() const { return cap && cap->isOpened(); }

private:
    DEFUN_WARN_UNUSED bool _get_frame(cv::Mat& frame);

    settings_pt s;

    double dt_valid;
    double dt_mean;

    CamInfo cam_info;
    CamInfo cam_desired;
    QString desired_name, active_name;

    struct camera_deleter final
    {
        void operator()(cv::VideoCapture* cap);
    };

    using camera_ptr = std::unique_ptr<cv::VideoCapture, camera_deleter>;

    camera_ptr cap;
};

} // ns impl

using impl::Camera;
using impl::CamInfo;
