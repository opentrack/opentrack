/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "camera.h"
#include "compat/sleep.hpp"

namespace impl {

QString Camera::get_desired_name() const
{
    return desired_name;
}

QString Camera::get_active_name() const
{
    return active_name;
}

void CamInfo::get_focal_length(f& fx) const
{
    using std::tan;
    using std::atan;
    using std::sqrt;

    const double diag_len = sqrt(double(res_x*res_x + res_y*res_y));
    const double aspect_x = res_x / diag_len;
    //const double aspect_y = res_y / diag_len;
    const double diag_fov = fov * M_PI/180;
    const double fov_x = 2*atan(tan(diag_fov*.5) * aspect_x);
    //const double fov_y = 2*atan(tan(diag_fov*.5) * aspect_y);
    fx = .5 / tan(fov_x * .5);
    //fy = .5 / tan(fov_y * .5);
    //static bool once = false; if (!once) { once = true; qDebug() << "f" << ret << "fov" << (fov * 180/M_PI); }
}

DEFUN_WARN_UNUSED bool Camera::get_info(CamInfo& ret) const
{
    if (cam_info.res_x == 0 || cam_info.res_y == 0)
        return false;
    ret = cam_info;
    return true;
}

DEFUN_WARN_UNUSED bool Camera::get_frame(double dt, cv::Mat& frame, CamInfo& info)
{
    bool new_frame = _get_frame(frame);

    // measure fps of valid frames
    static constexpr double RC = .1; // seconds
    const double alpha = dt/(dt + RC);
    dt_valid += dt;

    if (new_frame)
    {
        if (dt_mean < 2e-3)
            dt_mean = dt;
        else
            dt_mean = (1-alpha) * dt_mean + alpha * dt_valid;

        cam_info.fps = dt_mean > 2e-3 ? int(1 / dt_mean + .65) : 0;
        cam_info.res_x = frame.cols;
        cam_info.res_y = frame.rows;
        cam_info.fov = s.fov;

        info = cam_info;

        dt_valid = 0;
    }
    else
        qDebug() << "pt camera: can't get frame";
    return new_frame;
}

DEFUN_WARN_UNUSED bool Camera::start(int idx, int fps, int res_x, int res_y)
{
    if (idx >= 0 && fps >= 0 && res_x > 0 && res_y > 0)
    {
        if (!cap || cap->isOpened() ||
            cam_desired.idx != idx ||
            cam_desired.fps != fps ||
            cam_desired.res_x != res_x ||
            cam_desired.res_y != res_y)
        {
            qDebug() << "pt: opening camera";

            cam_desired.idx = idx;
            cam_desired.fps = fps;
            cam_desired.res_x = res_x;
            cam_desired.res_y = res_y;

            cap = camera_ptr(new cv::VideoCapture(cam_desired.idx));

            cap->set(cv::CAP_PROP_FRAME_WIDTH,  cam_desired.res_x);
            cap->set(cv::CAP_PROP_FRAME_HEIGHT, cam_desired.res_y);
            cap->set(cv::CAP_PROP_FPS, cam_desired.fps);

            if (cap->isOpened())
            {
                cam_info = CamInfo();
                cam_info.idx = cam_desired.idx;
                active_name = desired_name;

                return true;
            }
        }
    }

    return stop(), false;
}

void Camera::stop()
{
    cap = nullptr;
    desired_name = QString();
    active_name = QString();
    cam_info = CamInfo();
    cam_desired = CamInfo();
}

DEFUN_WARN_UNUSED bool Camera::_get_frame(cv::Mat& frame)
{
    if (cap && cap->isOpened())
    {
        for (int i = 0; i < 5 && !cap->read(frame); i++)
            portable::sleep(100);

        if (frame.empty())
            return false;

        return true;
    }
    return false;
}

void Camera::camera_deleter::operator()(cv::VideoCapture* cap)
{
    if (cap)
    {
        if (cap->isOpened())
            cap->release();
        std::default_delete<cv::VideoCapture>()(cap);
    }
}

} // ns impl
