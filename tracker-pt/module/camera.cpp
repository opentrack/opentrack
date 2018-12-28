/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "camera.h"
#include "frame.hpp"

#include "compat/sleep.hpp"
#include "compat/camera-names.hpp"
#include "compat/math-imports.hpp"

#include <opencv2/imgproc.hpp>

#include "cv/video-property-page.hpp"

#include <cstdlib>

namespace pt_module {

Camera::Camera(const QString& module_name) : s { module_name }
{
}

QString Camera::get_desired_name() const
{
    return desired_name;
}

QString Camera::get_active_name() const
{
    return active_name;
}

void Camera::show_camera_settings()
{
    const int idx = camera_name_to_index(s.camera_name);

    if (cap && cap->isOpened())
        video_property_page::show_from_capture(*cap, idx);
    else
        video_property_page::show(idx);
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
        const double dt = t.elapsed_seconds();
        t.start();

        // measure fps of valid frames
        constexpr double RC = .1; // seconds
        const double alpha = dt/(dt + RC);

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

bool Camera::start(int idx, int fps, int res_x, int res_y)
{
    if (idx >= 0 && fps >= 0 && res_x >= 0 && res_y >= 0)
    {
        if (cam_desired.idx != idx ||
            (int)cam_desired.fps != fps ||
            cam_desired.res_x != res_x ||
            cam_desired.res_y != res_y ||
            !cap || !cap->isOpened() || !cap->grab())
        {
            stop();

            desired_name = get_camera_names().value(idx);
            cam_desired.idx = idx;
            cam_desired.fps = fps;
            cam_desired.res_x = res_x;
            cam_desired.res_y = res_y;
            cam_desired.fov = fov;

            cap = camera_ptr(new cv::VideoCapture(idx));

            if (cam_desired.res_x > 0 && cam_desired.res_y > 0)
            {
                cap->set(cv::CAP_PROP_FRAME_WIDTH,  res_x);
                cap->set(cv::CAP_PROP_FRAME_HEIGHT, res_y);
            }

            if (fps > 0)
                cap->set(cv::CAP_PROP_FPS, fps);

            if (cap->isOpened())
            {
                cam_info = pt_camera_info();
                cam_info.idx = idx;
                dt_mean = 0;
                active_name = desired_name;

                cv::Mat tmp;

                if (get_frame_(tmp))
                {
                    t.start();
                    return true;
                }
            }

            cap = nullptr;
            return false;
        }

        return true;
    }

    stop();
    return false;
}

void Camera::stop()
{
    cap = nullptr;
    desired_name = QString{};
    active_name = QString{};
    cam_info = {};
    cam_desired = {};
}

bool Camera::get_frame_(cv::Mat& frame)
{
    if (cap && cap->isOpened())
    {
        for (unsigned i = 0; i < 10; i++)
        {
            if (cap->read(frame))
                return true;
            portable::sleep(50);
        }
    }
    return false;
}

void Camera::camera_deleter::operator()(cv::VideoCapture* cap)
{
    if (cap)
    {
        if (cap->isOpened())
            cap->release();
        delete cap;
    }
}

} // ns pt_module
