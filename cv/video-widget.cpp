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
        if (width < 1 || height < 1)
            return;

        if (_frame.cols != frame.cols || _frame.rows != frame.rows)
            _frame = cv::Mat(frame.rows, frame.cols, CV_8UC3);
        frame.copyTo(_frame);
        freshp = true;

        if (_frame2.cols != _frame.cols || _frame2.rows != _frame.rows)
            _frame2 = cv::Mat(_frame.rows, _frame.cols, CV_8UC4);

        if (_frame3.cols != width || _frame3.rows != height)
            _frame3 = cv::Mat(height, width, CV_8UC4);

        cv::cvtColor(_frame, _frame2, cv::COLOR_BGR2BGRA);

        const cv::Mat* img;

        if (_frame.cols != width || _frame.rows != height)
        {
            cv::resize(_frame2, _frame3, cv::Size(width, height), 0, 0, cv::INTER_NEAREST);

            img = &_frame3;
        }
        else
            img = &_frame2;

        const unsigned nbytes = 4 * img->rows * img->cols;

        vec.resize(nbytes);

        std::memcpy(vec.data(), img->data, nbytes);

        texture = QImage((const unsigned char*) vec.data(), width, height, QImage::Format_ARGB32);
    }
}

void cv_video_widget::update_image(const QImage& img)
{
    QMutexLocker l(&mtx);

    if (freshp)
        return;

    const unsigned nbytes = img.bytesPerLine() * img.height();

    vec.resize(nbytes);

    std::memcpy(vec.data(), img.constBits(), nbytes);

    texture = QImage((const unsigned char*) vec.data(), img.width(), img.height(), img.format());

    freshp = true;
}

void cv_video_widget::paintEvent(QPaintEvent*)
{
    QMutexLocker foo(&mtx);

    QPainter painter(this);

    double dpr = devicePixelRatioF();

    int W = iround(QWidget::width() * dpr);
    int H = iround(QWidget::height() * dpr);

    painter.drawImage(rect(), texture);

    if (texture.width() != W || texture.height() != H)
    {
        texture = QImage(W, H, QImage::Format_ARGB32);
        texture.setDevicePixelRatio(dpr);

        width = W, height = H;

        _frame = cv::Mat();
        _frame2 = cv::Mat();
        _frame3 = cv::Mat();
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

void cv_video_widget::get_preview_size(int& w, int& h)
{
    QMutexLocker l(&mtx);

    w = width, h = height;
}
