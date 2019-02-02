/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014-2016 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "video-widget.hpp"
#include "compat/check-visible.hpp"
#include "compat/math.hpp"

#include <cstring>
#include <opencv2/imgproc.hpp>

cv_video_widget::cv_video_widget(QWidget* parent) : QWidget(parent)
{
    texture = QImage(width(), height(), QImage::Format_ARGB32);
    texture.fill(Qt::gray);
    texture.setDevicePixelRatio(devicePixelRatioF());

    connect(&timer, SIGNAL(timeout()), this, SLOT(update_and_repaint()), Qt::DirectConnection);
    timer.start(65);
}

void cv_video_widget::update_image(const cv::Mat& frame)
{
    QMutexLocker l(&mtx);

    if (!freshp)
    {
        if (W < 1 || H < 1 || frame.rows < 1 || frame.cols < 1)
            return;

        freshp = true;

        if (frame2.cols != frame.cols || frame2.rows != frame.rows)
            frame2 = cv::Mat(frame.rows, frame.cols, CV_8UC4);

        if (frame3.cols != W || frame3.rows != H)
            frame3 = cv::Mat(H, W, CV_8UC4);

        switch (frame.channels())
        {
        case 1:
            cv::cvtColor(frame, frame2, cv::COLOR_GRAY2BGRA);
            break;
        case 3:
            cv::cvtColor(frame, frame2, cv::COLOR_BGR2BGRA);
            break;
        case 4:
            frame2.setTo(frame);
            break;
        default:
            *(volatile int*)nullptr = 0; // NOLINT(clang-analyzer-core.NullDereference)
            unreachable();
        }

        const cv::Mat* img;

        if (frame2.cols != W || frame2.rows != H)
        {
            cv::resize(frame2, frame3, cv::Size(W, H), 0, 0, cv::INTER_NEAREST);
            img = &frame3;
        }
        else
            img = &frame2;

        const unsigned nbytes = unsigned(4 * img->rows * img->cols);
        vec.resize(nbytes);
        std::memcpy(vec.data(), img->data, nbytes);

        texture = QImage((const unsigned char*) vec.data(), W, H, QImage::Format_ARGB32);
        texture.setDevicePixelRatio(devicePixelRatioF());
    }
}

void cv_video_widget::update_image(const QImage& img)
{
    QMutexLocker l(&mtx);

    if (freshp)
        return;
    freshp = true;

    const unsigned nbytes = unsigned(img.bytesPerLine() * img.height());
    vec.resize(nbytes);
    std::memcpy(vec.data(), img.constBits(), nbytes);

    texture = QImage((const unsigned char*) vec.data(), img.width(), img.height(), img.format());
    texture.setDevicePixelRatio(devicePixelRatioF());
}

void cv_video_widget::paintEvent(QPaintEvent*)
{
    QMutexLocker foo(&mtx);

    QPainter painter(this);

    double dpr = devicePixelRatioF();
    W = iround(width() * dpr);
    H = iround(height() * dpr);

    painter.drawImage(rect(), texture);

    if (texture.width() != W || texture.height() != H)
    {
        texture = QImage(W, H, QImage::Format_ARGB32);
        texture.setDevicePixelRatio(dpr);

        frame2 = cv::Mat();
        frame3 = cv::Mat();
    }
}

void cv_video_widget::update_and_repaint()
{
    if (!check_is_visible())
        return;

    QMutexLocker l(&mtx);

    if (freshp)
    {
        freshp = false;
        repaint();
    }
}

void cv_video_widget::resizeEvent(QResizeEvent*)
{
    QMutexLocker l(&mtx);
    W = iround(width() * devicePixelRatioF());
    H = iround(height() * devicePixelRatioF());
}

void cv_video_widget::get_preview_size(int& w, int& h)
{
    QMutexLocker l(&mtx);
    w = W; h = H;
}
