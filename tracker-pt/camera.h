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
    CamInfo() : res_x(0), res_y(0), fps(0), idx(-1) {}

    int res_x;
    int res_y;
    int fps;
    int idx;
};

class Camera final
{
public:
        Camera() : dt_valid(0), dt_mean(0) {}
        ~Camera();

        void start();
        void stop();
        void restart() { stop(); start(); }

        // calls corresponding template methods and reinitializes frame rate calculation
        void set_device(const QString& name);
        void set_fps(int fps);
        void set_res(int x_res, int y_res);

        DEFUN_WARN_UNUSED bool get_frame(double dt, cv::Mat* frame);
        DEFUN_WARN_UNUSED bool get_info(CamInfo &ret);

        CamInfo get_desired() const { return cam_desired; }
        QString get_desired_name() const;
        QString get_active_name() const;

        cv::VideoCapture& operator*() { return *cap; }
        const cv::VideoCapture& operator*() const { return *cap; }
        cv::VideoCapture* operator->() { return cap.get(); }
        const cv::VideoCapture* operator->() const { return cap.get(); }
        operator bool() const { return cap && cap->isOpened(); }

private:
        DEFUN_WARN_UNUSED bool _get_frame(cv::Mat* frame);

        double dt_valid;
        double dt_mean;

        CamInfo cam_info;
        CamInfo cam_desired;
        QString desired_name, active_name;

        struct camera_deleter final
        {
            void operator()(cv::VideoCapture* cap)
            {
                if (cap)
                {
                    if (cap->isOpened())
                        cap->release();
                    static const std::default_delete<cv::VideoCapture> deleter;
                    deleter(cap);
                }
            }
        };

        using camera_ptr = std::unique_ptr<cv::VideoCapture, camera_deleter>;

        std::unique_ptr<cv::VideoCapture, camera_deleter> cap;
};
