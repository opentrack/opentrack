/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014 Stanislaw Halik <sthalik@misaki.pl>
 * Copyright (c) 2016 fred41
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

class WiiMoteMonitorWidget : public QWidget
{
    Q_OBJECT
public:
    WiiMoteMonitorWidget(QWidget *parent) :
        QWidget(parent)
    {
        setAttribute(Qt::WA_OpaquePaintEvent);
    }
    void update_image(const std::vector<cv::Vec2f>& points);
protected slots:
    void paintEvent( QPaintEvent* e ) {
        QPainter p(this);
        p.drawImage(0, 0, image_);
    }
private:
    cv::Mat frame;
    QImage image_;
};
