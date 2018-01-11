/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "compat/ndebug-guard.hpp"
#include "pt-api.hpp"

#include "compat/util.hpp"
#include "compat/timer.hpp"

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include <memory>
#include <tuple>
#include <QString>

struct Camera final : pt_camera
{
    Camera();

    pt_camera_open_status start(int idx, int fps, int res_x, int res_y) override;
    void stop() override;

    result get_frame(cv::Mat& frame) override;
    result get_info() const override;

    pt_camera_info get_desired() const override { return cam_desired; }
    QString get_desired_name() const override;
    QString get_active_name() const override;

    operator bool() const override { return cap && cap->isOpened(); }

    void set_fov(double value) override { fov = value; }

    void show_camera_settings() override;

private:
    warn_result_unused bool _get_frame(cv::Mat& frame);

    double dt_mean, fov;

    Timer t;

    pt_camera_info cam_info;
    pt_camera_info cam_desired;
    QString desired_name, active_name;

    struct camera_deleter final
    {
        void operator()(cv::VideoCapture* cap);
    };

    using camera_ptr = std::unique_ptr<cv::VideoCapture, camera_deleter>;

    camera_ptr cap;

    pt_settings s;

    static constexpr double dt_eps = 1./384;
};
