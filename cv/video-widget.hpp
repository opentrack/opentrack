/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014-2016 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include <memory>
#include <vector>

#include <opencv2/core.hpp>

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QtEvents>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QSize>
#include <QDebug>

class cv_video_widget final : public QWidget
{
    Q_OBJECT

public:
    cv_video_widget(QWidget *parent);
    void update_image(const cv::Mat& frame);
    void update_image(const QImage& image);
    void get_preview_size(int& w, int& h);
private slots:
    void paintEvent(QPaintEvent*) override;
    void update_and_repaint();
private:
    QMutex mtx { QMutex::Recursive };
    QImage texture;
    std::vector<unsigned char> vec;
    QTimer timer;
    cv::Mat frame1, frame2, frame3;
    int width = 320, height = 240;
    bool freshp = false;
};
