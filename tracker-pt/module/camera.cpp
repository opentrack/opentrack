/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "camera.h"
#include "frame.hpp"

#include "compat/math-imports.hpp"

#include <opencv2/core.hpp>

namespace pt_module {

Camera::Camera(const QString& module_name) : s { module_name }
{
}

QString Camera::get_desired_name() const
{
    return cam_desired.name;
}

QString Camera::get_active_name() const
{
    return cam_info.name;
}

void Camera::show_camera_settings()
{
    if (cap)
        (void)cap->show_dialog();
}

Camera::result Camera::get_info() const
{
    if (cam_info.res_x == 0 || cam_info.res_y == 0)
        return { false, pt_camera_info() };
    else
        return { true, cam_info };
}

Camera::result Camera::get_frame(pt_frame& frame_)
{
    cv::Mat& frame = frame_.as<Frame>()->mat;

    const bool new_frame = get_frame_(frame);

    if (new_frame)
    {
        const f dt = (f)t.elapsed_seconds();
        t.start();

        // measure fps of valid frames
        constexpr f RC = f{1}/10; // seconds
        const f alpha = dt/(dt + RC);

        if (dt_mean < dt_eps)
            dt_mean = dt;
        else
            dt_mean = (1-alpha) * dt_mean + alpha * dt;

        cam_info.fps = dt_mean > dt_eps ? 1 / dt_mean : 0;
        cam_info.res_x = frame.cols;
        cam_info.res_y = frame.rows;
        cam_info.fov = fov;

        return { true, cam_info };
    }
    else
        return { false, {} };
}

bool Camera::start(const QString& name, int fps, int res_x, int res_y)
{
    if (fps >= 0 && res_x >= 0 && res_y >= 0)
    {
        if (cam_desired.name != name ||
            (int)cam_desired.fps != fps ||
            cam_desired.res_x != res_x ||
            cam_desired.res_y != res_y ||
            !cap || !cap->is_open())
        {
            stop();

            cam_desired.name = name;
            cam_desired.fps = fps;
            cam_desired.res_x = res_x;
            cam_desired.res_y = res_y;
            cam_desired.fov = fov;

            cap = video::make_camera(name);

            if (!cap)
                goto fail;

            camera::info info {};
            info.fps = fps;
            info.width = res_x;
            info.height = res_y;

            if (!cap->start(info))
                goto fail;

            cam_info = pt_camera_info();
            cam_info.name = name;
            dt_mean = 0;

            cv::Mat tmp;

            if (!get_frame_(tmp))
                goto fail;

            t.start();
        }
    }

    return true;

fail:
    stop();
    return false;
}

void Camera::stop()
{
    cap = nullptr;
    cam_info = {};
    cam_desired = {};
}

bool Camera::get_frame_(cv::Mat& img)
{
    if (cap && cap->is_open())
    {
        auto [ frame, ret ] = cap->get_frame();
        if (ret)
        {
            img = cv::Mat(frame.height, frame.width, CV_8UC(frame.channels), (void*)frame.data, frame.stride);
            return true;
        }
    }

    return false;
}

} // ns pt_module
