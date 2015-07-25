/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "camera.h"
#include <string>
#include <QDebug>
#include "opentrack-compat/sleep.hpp"

void Camera::set_device_index(int index)
{
    if (desired_index != index)
    {
        desired_index = index;
        _set_device_index();

        // reset fps
        dt_valid = 0;
        dt_mean = 0;
        active_index = index;
    }
}

void Camera::set_fps(int fps)
{
    if (cam_desired.fps != fps)
    {
        cam_desired.fps = fps;
        _set_fps();
    }
}

void Camera::set_res(int x_res, int y_res)
{
    if (cam_desired.res_x != x_res || cam_desired.res_y != y_res)
    {
        cam_desired.res_x = x_res;
        cam_desired.res_y = y_res;
        _set_res();
    }
}

CamInfo Camera::get_info()
{
    if (cam_info.res_x == 0 || cam_info.res_y == 0)
    {
        cv::Mat tmp;
        _get_frame(&tmp);
    }
    return cam_info;
}

bool Camera::get_frame(float dt, cv::Mat* frame)
{
    bool new_frame = _get_frame(frame);
    // measure fps of valid frames
    const float dt_smoothing_const = 0.9;
    dt_valid += dt;
    if (new_frame)
    {
        dt_mean = dt_smoothing_const * dt_mean + (1.0 - dt_smoothing_const) * dt_valid;
        cam_info.fps = dt_mean > 1e-3 ? 1.0 / dt_mean : 0;
        dt_valid = 0;
    }
    return new_frame;
}

void CVCamera::start()
{
    if (cap)
        delete cap;
    cap = new cv::VideoCapture(desired_index);
    _set_res();
    _set_fps();
    // extract camera info
    if (cap->isOpened())
    {
        active = true;
        active_index = desired_index;
        cam_info.res_x = 0;
        cam_info.res_y = 0;
    } else {
        delete cap;
        cap = nullptr;
    }
}

void CVCamera::stop()
{
    if (cap)
    {
        cap->release();
        delete cap;
        cap = nullptr;
        // give opencv time to exit camera threads, etc.
        portable::sleep(500);
    }
    active = false;
}

bool CVCamera::_get_frame(cv::Mat* frame)
{
    if (cap && cap->isOpened())
    {
        cv::Mat img;
        for (int i = 0; i < 100 && !cap->read(img); i++)
            ;;

        if (img.empty())
            return false;

        *frame = img;
        cam_info.res_x = img.cols;
        cam_info.res_y = img.rows;
        return true;
    }
    return false;
}

void CVCamera::_set_fps()
{
    if (cap) cap->set(CV_CAP_PROP_FPS, cam_desired.fps);
}

void CVCamera::_set_res()
{
    if (cap)
    {
        cap->set(CV_CAP_PROP_FRAME_WIDTH,  cam_desired.res_x);
        cap->set(CV_CAP_PROP_FRAME_HEIGHT, cam_desired.res_y);
    }
}
void CVCamera::_set_device_index()
{
    if (cap)
    {
        cap->release();
        delete cap;
    }
    cap = new cv::VideoCapture(desired_index);
}
