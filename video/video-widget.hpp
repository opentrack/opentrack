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
#include <atomic>
#include <tuple>

#include <QWidget>
#include <QImage>
#include <QTimer>

#include <QRecursiveMutex>

struct OTR_VIDEO_EXPORT video_widget : QWidget
{
    video_widget(QWidget* parent = nullptr);

    void update_image(const QImage& image);
    std::tuple<int, int> preview_size() const;
    void resizeEvent(QResizeEvent*) override;
    void paintEvent(QPaintEvent*) override;
    void draw_image();
    bool fresh() const;

protected:
    mutable QRecursiveMutex mtx;
    QImage texture;
    std::vector<unsigned char> vec;
    void set_fresh(bool x);
    void set_image(const unsigned char* src, int width, int height, int stride, QImage::Format fmt);

private:
    void init_image_nolock();
    QTimer timer;

    std::atomic<QSize> size_ = QSize(320, 240);
    std::atomic<bool> fresh_ { false };

    static_assert(decltype(fresh_)::is_always_lock_free);
};
