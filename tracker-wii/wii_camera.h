/*
 * Copyright (c) 2017-2018 Wei Shuai <cpuwolf@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "pt-api.hpp"

#include "compat/timer.hpp"

#include <functional>
#include <memory>
#include <tuple>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include <QString>

#include <wiimote.h>
#include "wii_frame.hpp"

namespace pt_module {

struct WIICamera final : pt_camera
{
    WIICamera(const QString& module_name);
    ~WIICamera() override;

    bool start(int idx, int fps, int res_x, int res_y) override;
    void stop() override;

    result get_frame(pt_frame& Frame) override;
    result get_info() const override;

    pt_camera_info get_desired() const override { return cam_desired; }
    QString get_desired_name() const override;
    QString get_active_name() const override;

    void set_fov(f value) override { (void)value; }
    void show_camera_settings() override;

private:
    static void on_state_change(wiimote &remote,
                                state_change_flags changed,
                                const wiimote_state &new_state);
    wii_camera_status pair();
    wii_camera_status get_frame_(cv::Mat& Frame);
    bool get_points(struct wii_info& wii);
    void get_status(struct wii_info& wii);

    std::unique_ptr<wiimote> m_pDev;

    pt_camera_info cam_info;
    pt_camera_info cam_desired;

    struct { float p = 0, r = 0; } horizon;

    pt_settings s;
};

} // ns pt_module
