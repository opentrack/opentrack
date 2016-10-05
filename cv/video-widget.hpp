/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014-2016 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include <opencv2/core/core.hpp>
#include <memory>
#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

class cv_video_widget final : public QWidget
{
    Q_OBJECT
public:
    cv_video_widget(QWidget *parent);
    void update_image(const cv::Mat &frame);
protected slots:
    void paintEvent(QPaintEvent*) override;
    void update_and_repaint();
private:
    QMutex mtx;
    QImage texture;
    QTimer timer;
    cv::Mat _frame, _frame2, _frame3;
    bool freshp;
};
