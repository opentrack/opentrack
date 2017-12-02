/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "camera.h"
#include "compat/sleep.hpp"
#include "compat/camera-names.hpp"
#include "compat/math-imports.hpp"

constexpr double Camera::dt_eps;

QString Camera::get_desired_name() const
{
    return desired_name;
}

QString Camera::get_active_name() const
{
    return active_name;
}

double CamInfo::get_focal_length() const
{
    const double diag_len = sqrt(double(res_x*res_x + res_y*res_y));
    const double aspect_x = res_x / diag_len;
    //const double aspect_y = res_y / diag_len;
    const double diag_fov = fov * M_PI/180;
    const double fov_x = 2*atan(tan(diag_fov*.5) * aspect_x);
    //const double fov_y = 2*atan(tan(diag_fov*.5) * aspect_y);
    const double fx = .5 / tan(fov_x * .5);
    return fx;
    //fy = .5 / tan(fov_y * .5);
    //static bool once = false; if (!once) { once = true; qDebug() << "f" << ret << "fov" << (fov * 180/M_PI); }
}

warn_result_unused Camera::result Camera::get_info() const
{
    if (cam_info.res_x == 0 || cam_info.res_y == 0)
        return result(false, CamInfo());
    return result(true, cam_info);
}

warn_result_unused Camera::result Camera::get_frame(cv::Mat& frame)
{
    const bool new_frame = _get_frame(frame);

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

        return result(true, cam_info);
    }
    else
        return result(false, CamInfo());
}

warn_result_unused Camera::open_status Camera::start(int idx, int fps, int res_x, int res_y)
{
    if (idx >= 0 && fps >= 0 && res_x >= 0 && res_y >= 0)
    {
        if (cam_desired.idx != idx ||
            cam_desired.fps != fps ||
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

            cap = camera_ptr(new cv::VideoCapture(cam_desired.idx));

            if (cam_desired.res_x)
                cap->set(cv::CAP_PROP_FRAME_WIDTH,  cam_desired.res_x);
            if (cam_desired.res_y)
                cap->set(cv::CAP_PROP_FRAME_HEIGHT, cam_desired.res_y);
            if (cam_desired.fps)
                cap->set(cv::CAP_PROP_FPS, cam_desired.fps);

            if (cap->isOpened() && cap->grab())
            {
                cam_info = CamInfo();
                active_name = QString();
                cam_info.idx = idx;
                dt_mean = 0;
                active_name = desired_name;

                t.start();

                return open_ok_change;
            }
            else
            {
                stop();
                return open_error;
            }
        }

        return open_ok_no_change;
    }

    stop();
    return open_error;
}

void Camera::stop()
{
    cap = nullptr;
    desired_name = QString();
    active_name = QString();
    cam_info = CamInfo();
    cam_desired = CamInfo();
}

warn_result_unused bool Camera::_get_frame(cv::Mat& frame)
{
    if (cap && cap->isOpened())
    {
        for (int i = 0; i < 5; i++)
        {
            if (cap->read(frame))
                return true;
            portable::sleep(1);
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
