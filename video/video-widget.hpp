/* Copyright (c) 2014-2016, 2019 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "compat/math.hpp"
#include "export.hpp"

#include <vector>

#include <QWidget>
#include <QTimer>
#include <QMutex>

class OTR_VIDEO_EXPORT video_widget : public QWidget
{
    Q_OBJECT

public:
    video_widget(QWidget* parent = nullptr);

    void update_image(const QImage& image);
    void get_preview_size(int& w, int& h);
    void resizeEvent(QResizeEvent*) override;
protected slots:
    void paintEvent(QPaintEvent*) override;
    void update_and_repaint();
private:
    QTimer timer;

protected:
    QMutex mtx { QMutex::Recursive };
    QImage texture;
    std::vector<unsigned char> vec;

    bool freshp = false;

    int W = iround(QWidget::width() * devicePixelRatioF());
    int H = iround(QWidget::height() * devicePixelRatioF());

    void init_image_nolock();
};
