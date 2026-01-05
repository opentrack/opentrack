/*
* Copyright (c) 2017-2018 Wei Shuai <cpuwolf@gmail.com>
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*/

#include "wii_frame.hpp"

#include "compat/math.hpp"

#include <cstring>
#include <tuple>

#include <opencv2/imgproc.hpp>

using namespace pt_module;

void WIIPreview::set_last_frame(const pt_frame& frame_)
{
    const struct wii_info& wii = frame_.as_const<WIIFrame>()->wii;
    const cv::Mat& frame = frame_.as_const<const WIIFrame>()->mat;

    status = wii.status;

    if (frame.channels() != 3)
        eval_once(qDebug() << "tracker/pt: camera frame depth: 3 !=" << frame.channels());

    const bool need_resize = frame.cols != frame_out.cols || frame.rows != frame_out.rows;
    if (need_resize)
        cv::resize(frame, frame_copy, cv::Size(frame_out.cols, frame_out.rows), 0, 0, cv::INTER_NEAREST);
    else
        frame.copyTo(frame_copy);
}

WIIPreview::WIIPreview(int w, int h)
{
    ensure_size(frame_out, w, h, CV_8UC4);
    ensure_size(frame_copy, w, h, CV_8UC3);

    frame_copy.setTo(cv::Scalar(0, 0, 0));
}

QImage WIIPreview::get_bitmap()
{
	switch (status) {
	case wii_cam_wait_for_dongle:
		return QImage(":/Resources/usb.png");
	case wii_cam_wait_for_sync:
		return QImage(":/Resources/sync.png");
	case wii_cam_wait_for_connect:
		return QImage(":/Resources/on.png");
	}
    int stride = (int)frame_out.step.p[0];

    if (stride < 64 || stride < frame_out.cols * 4)
    {
        eval_once(qDebug() << "bad stride" << stride
                           << "for bitmap size" << frame_copy.cols << frame_copy.rows);
        return QImage();
    }

    cv::cvtColor(frame_copy, frame_out, cv::COLOR_BGR2BGRA);

    return QImage((const unsigned char*) frame_out.data,
                  frame_out.cols, frame_out.rows,
                  stride,
                  QImage::Format_ARGB32);
}

void WIIPreview::draw_head_center(f x, f y)
{
    double px_, py_;

    std::tie(px_, py_) = to_pixel_pos(x, y, frame_copy.cols, frame_copy.rows);

    int px = iround(px_), py = iround(py_);

    constexpr int len = 9;

    static const cv::Scalar color(0, 255, 255);
    cv::line(frame_copy,
             cv::Point(px - len, py),
             cv::Point(px + len, py),
             color, 1);
    cv::line(frame_copy,
             cv::Point(px, py - len),
             cv::Point(px, py + len),
             color, 1);
}

void WIIPreview::ensure_size(cv::Mat& frame, int w, int h, int type)
{
    if (frame.cols != w || frame.rows != h || frame.type() != type)
        frame = cv::Mat(h, w, type);
}
