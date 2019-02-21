#include "video-widget.hpp"

#include "compat/check-visible.hpp"
#include "compat/math.hpp"

#include <cstddef>
#include <cstring>

#include <QPainter>

void video_widget::init_image_nolock()
{
    texture = QImage(W, H, QImage::Format_ARGB32);
    texture.setDevicePixelRatio(devicePixelRatioF());
}

video_widget::video_widget(QWidget* parent) : QWidget(parent)
{
    W = width(); H = height();
    init_image_nolock(); texture.fill(Qt::gray);

    connect(&timer, &QTimer::timeout, this, &video_widget::update_and_repaint, Qt::DirectConnection);
    timer.start(65);
}

void video_widget::update_image(const QImage& img)
{
    if (freshp.load(std::memory_order_relaxed))
        return;

    QMutexLocker l(&mtx);

    unsigned nbytes = (unsigned)(img.bytesPerLine() * img.height());
    vec.resize(nbytes); vec.shrink_to_fit();
    std::memcpy(vec.data(), img.constBits(), nbytes);

    texture = QImage((const unsigned char*) vec.data(), img.width(), img.height(), img.bytesPerLine(), img.format());
    texture.setDevicePixelRatio(devicePixelRatioF());

    freshp.store(true, std::memory_order_relaxed);
}

void video_widget::paintEvent(QPaintEvent*)
{
    QMutexLocker foo(&mtx);

    QPainter painter(this);
    painter.drawImage(rect(), texture);
}

void video_widget::update_and_repaint()
{
    if (!freshp.load(std::memory_order_relaxed))
        return;

    if (!check_is_visible())
        return;

    QMutexLocker l(&mtx);
    repaint();
    freshp.store(false, std::memory_order_relaxed);
}

void video_widget::resizeEvent(QResizeEvent*)
{
    QMutexLocker l(&mtx);
    double dpr = devicePixelRatioF();
    W = iround(width() * dpr);
    H = iround(height() * dpr);
    init_image_nolock();
}

void video_widget::get_preview_size(int& w, int& h)
{
    QMutexLocker l(&mtx);
    w = W; h = H;
}

