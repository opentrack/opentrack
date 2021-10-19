/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "pt-api.hpp"
#include "compat/timer.hpp"
#include "video/camera.hpp"

#include <memory>

#include <QString>

namespace pt_module {

struct Camera final : pt_camera
{
    Camera(const QString& module_name);

    bool start(const pt_settings& s) override;
    void stop() override;

    result get_frame(pt_frame& Frame) override;
    result get_info() const override;

    pt_camera_info get_desired() const override { return cam_desired; }
    QString get_desired_name() const override;
    QString get_active_name() const override;

    void set_fov(f value) override { fov = value; }
    void show_camera_settings() override;

private:
    using camera = video::impl::camera;

    [[nodiscard]] bool get_frame_(cv::Mat& frame);

    f dt_mean = 0, fov = 30;
    Timer t;
    pt_camera_info cam_info;
    pt_camera_info cam_desired;

    std::unique_ptr<camera> cap;
    pt_settings s;

    static constexpr f dt_eps = f{1}/256;
};

} // ns pt_module
