/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "compat/util.hpp"

#include <opencv2/core/core.hpp>
#include <memory>
#include <opencv2/videoio.hpp>
#include <string>
#include <QString>

struct CamInfo
{
    CamInfo() : res_x(0), res_y(0), fps(0) {}

    int res_x;
    int res_y;
    int fps;
};

// ----------------------------------------------------------------------------
// Base class for cameras, calculates the frame rate
class Camera
{
public:
        Camera() : dt_valid(0), dt_mean(0), desired_index(0), active_index(-1) {}
        virtual ~Camera() = 0;

        // start/stop capturing
        virtual void start() = 0;
        virtual void stop() = 0;
        void restart() { stop(); start(); }

        // calls corresponding template methods and reinitializes frame rate calculation
        void set_device(const QString& name);
        void set_fps(int fps);
        void set_res(int x_res, int y_res);

        // gets a frame from the camera, dt: time since last call in seconds
        DEFUN_WARN_UNUSED bool get_frame(double dt, cv::Mat* frame);

        // WARNING: returned references are valid as long as object
        DEFUN_WARN_UNUSED bool get_info(CamInfo &ret);
        CamInfo get_desired() const { return cam_desired; }
        QString get_desired_name() const;
protected:
        // get a frame from the camera
        DEFUN_WARN_UNUSED virtual bool _get_frame(cv::Mat* frame) = 0;

        // update the camera using cam_desired, write res and f to cam_info if successful
        virtual void _set_device_index() = 0;
        virtual void _set_fps() = 0;
        virtual void _set_res() = 0;
private:
        double dt_valid;
        double dt_mean;
protected:
        CamInfo cam_info;
        CamInfo cam_desired;
        QString desired_name;
        int desired_index;
        int active_index;
};

// ----------------------------------------------------------------------------
// camera based on OpenCV's videoCapture
class CVCamera final : public Camera
{
public:
    CVCamera() : cap(NULL) {}
    ~CVCamera() { stop(); }

    void start() override;
    void stop() override;

    operator cv::VideoCapture*() { return cap; }
protected:
    bool _get_frame(cv::Mat* frame) override;
    void _set_fps() override;
    void _set_res() override;
    void _set_device_index() override;
private:
    cv::VideoCapture* cap;
};

enum RotationType
{
    CLOCKWISE = 0,
    ZERO = 1,
    COUNTER_CLOCKWISE = 2
};
