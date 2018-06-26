/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "pt-api.hpp"
#include "compat/timer.hpp"

#include <memory>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include <QString>

namespace pt_module {

struct Camera final : pt_camera
{
    Camera(const QString& module_name);

    bool start(int idx, int fps, int res_x, int res_y) override;
    void stop() override;

    result get_frame(pt_frame& Frame) override;
    result get_info() const override;

    pt_camera_info get_desired() const override { return cam_desired; }
    QString get_desired_name() const override;
    QString get_active_name() const override;

    void set_fov(double value) override { fov = value; }
    void show_camera_settings() override;

private:
    cc_warn_unused_result bool _get_frame(cv::Mat& Frame);

    double dt_mean = 0, fov = 30;
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

    static constexpr inline double dt_eps = 1./384;
};

} // ns pt_module
