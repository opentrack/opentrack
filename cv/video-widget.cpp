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
    connect(&timer, SIGNAL(timeout()), this, SLOT(update_and_repaint()), Qt::DirectConnection);
    timer.start(65);
}

void cv_video_widget::update_image(const cv::Mat& frame)
{
    QMutexLocker l(&mtx);

    if (!freshp)
    {
        if (width < 1 || height < 1 || frame.rows < 1 || frame.cols < 1)
            return;

        freshp = true;

        if (frame2.cols != frame.cols || frame2.rows != frame.rows)
            frame2 = cv::Mat(frame.rows, frame.cols, CV_8UC4);

        if (frame3.cols != width || frame3.rows != height)
            frame3 = cv::Mat(height, width, CV_8UC4);

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

        if (frame2.cols != width || frame2.rows != height)
        {
            cv::resize(frame2, frame3, cv::Size(width, height), 0, 0, cv::INTER_NEAREST);
            img = &frame3;
        }
        else
            img = &frame2;

        const unsigned nbytes = unsigned(4 * img->rows * img->cols);
        vec.resize(nbytes);
        std::memcpy(vec.data(), img->data, nbytes);

        texture = QImage((const unsigned char*) vec.data(), width, height, QImage::Format_ARGB32);
        double dpr = devicePixelRatioF();
        texture.setDevicePixelRatio(dpr);
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
    double dpr = devicePixelRatioF();
    texture.setDevicePixelRatio(dpr);
}

void cv_video_widget::paintEvent(QPaintEvent*)
{
    QMutexLocker foo(&mtx);

    QPainter painter(this);

    double dpr = devicePixelRatioF();

    width = iround(QWidget::width() * dpr);
    height = iround(QWidget::height() * dpr);

    painter.drawImage(rect(), texture);

    if (texture.width() != width || texture.height() != height)
    {
        texture = QImage(width, height, QImage::Format_ARGB32);
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
    width  = iround(QWidget::width() * devicePixelRatioF());
    height = iround(QWidget::height() * devicePixelRatioF());
}

void cv_video_widget::get_preview_size(int& w, int& h)
{
    QMutexLocker l(&mtx);
    w = width; h = height;
}
