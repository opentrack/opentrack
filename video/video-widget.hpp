/* Copyright (c) 2014-2016, 2019 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "compat/math.hpp"

#include <vector>

#ifdef OTR_VIDEO_HAS_OPENCV
#   include <opencv2/core.hpp>
#endif

#include <QWidget>
#include <QTimer>
#include <QMutex>

class cv_video_widget final : public QWidget
{
    Q_OBJECT

public:
    cv_video_widget(QWidget *parent);

#ifdef OTR_VIDEO_HAS_OPENCV
    void update_image(const cv::Mat& frame);
#endif
    void update_image(const QImage& image);
    void get_preview_size(int& w, int& h);
    void resizeEvent(QResizeEvent*) override;
private slots:
    void paintEvent(QPaintEvent*) override;
    void update_and_repaint();
private:
    QMutex mtx { QMutex::Recursive };
    QImage texture;
    std::vector<unsigned char> vec;
    QTimer timer;

#ifdef OTR_VIDEO_HAS_OPENCV
    cv::Mat frame2, frame3;
#endif
    bool freshp = false;

    int W  = iround(QWidget::width() * devicePixelRatioF());
    int H = iround(QWidget::height() * devicePixelRatioF());

    void init_image_nolock();
};
