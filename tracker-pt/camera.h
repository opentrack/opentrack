/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "compat/ndebug-guard.hpp"

#undef NDEBUG
#include <cassert>

#include "compat/util.hpp"
#include "compat/timer.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/videoio.hpp>

#include <memory>
#include <tuple>
#include <QString>

struct CamInfo final
{
    CamInfo() : fov(0), fps(0), res_x(0), res_y(0), idx(-1) {}
    double get_focal_length() const;

    double fov;
    double fps;

    int res_x;
    int res_y;
    int idx;
};

struct Camera final
{
    enum open_status : unsigned { open_error, open_ok_no_change, open_ok_change };

    using result = std::tuple<bool, CamInfo>;

    Camera() : dt_mean(0), fov(0) {}

    warn_result_unused open_status start(int idx, int fps, int res_x, int res_y);
    void stop();

    warn_result_unused result get_frame(cv::Mat& frame);
    warn_result_unused result get_info() const;

    CamInfo get_desired() const { return cam_desired; }
    QString get_desired_name() const;
    QString get_active_name() const;

    cv::VideoCapture& operator*() { assert(cap); return *cap; }
    const cv::VideoCapture& operator*() const { assert(cap); return *cap; }
    cv::VideoCapture* operator->() { assert(cap); return cap.get(); }
    const cv::VideoCapture* operator->() const { return cap.get(); }
    operator bool() const { return cap && cap->isOpened(); }

    void set_fov(double value) { fov = value; }

private:
    warn_result_unused bool _get_frame(cv::Mat& frame);

    double dt_mean;
    double fov;

    Timer t;

    CamInfo cam_info;
    CamInfo cam_desired;
    QString desired_name, active_name;

    struct camera_deleter final
    {
        void operator()(cv::VideoCapture* cap);
    };

    using camera_ptr = std::unique_ptr<cv::VideoCapture, camera_deleter>;

    camera_ptr cap;

    static constexpr double dt_eps = 1./384;
};
