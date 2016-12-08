/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "camera.h"
#include <string>
#include <QDebug>

QString Camera::get_desired_name() const
{
    return desired_name;
}

QString Camera::get_active_name() const
{
    return active_name;
}

DEFUN_WARN_UNUSED bool Camera::get_info(CamInfo& ret)
{
    if (cam_info.res_x == 0 || cam_info.res_y == 0)
        return false;
    ret = cam_info;
    return true;
}

bool Camera::get_frame(double dt, cv::Mat* frame)
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
                cam_info.idx = cam_desired.idx;
                cam_info.res_x = 0;
                cam_info.res_y = 0;
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

bool Camera::_get_frame(cv::Mat* frame)
{
    if (cap && cap->isOpened())
    {
        for (int i = 0; i < 100 && !cap->read(*frame); i++)
            ;;

        if (frame->empty())
            return false;

        cam_info.res_x = frame->cols;
        cam_info.res_y = frame->rows;
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
