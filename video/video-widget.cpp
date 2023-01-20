#include "video-widget.hpp"

#include "compat/check-visible.hpp"
#include "compat/math.hpp"

#include <cstring>

#include <QPainter>
#include <QtAlgorithms>
#include <QDebug>

void video_widget::init_image_nolock()
{
    double dpi = devicePixelRatioF();
    size_.store({ iround(width() * dpi), iround(height() * dpi) }, std::memory_order_release);
}

video_widget::video_widget(QWidget* parent) : QWidget(parent)
{
    if (parent)
        setFixedSize(parent->size());
    else
        setFixedSize(320, 240);
    init_image_nolock();
    connect(&timer, &QTimer::timeout, this, &video_widget::draw_image, Qt::DirectConnection);
    timer.start(15);
}

void video_widget::update_image(const QImage& img)
{
    if (fresh())
        return;

    set_image(img.constBits(), img.width(), img.height(),
              img.bytesPerLine(), img.format());
    set_fresh(true);
}

void video_widget::set_image(const unsigned char* src, int width, int height, int stride, QImage::Format fmt)
{
    QMutexLocker l(&mtx);

    texture = QImage();
    unsigned nbytes = (unsigned)(stride * height);
    vec.resize(nbytes); vec.shrink_to_fit();
    std::memcpy(vec.data(), src, nbytes);
    texture = QImage((const unsigned char*)vec.data(), width, height, stride, fmt);
}

void video_widget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    QMutexLocker l(&mtx);
    painter.drawImage(rect(), texture);
}

void video_widget::draw_image()
{
    if (!fresh())
        return;

    if (!check_is_visible())
        return;

    repaint();
    set_fresh(false);
}

void video_widget::resizeEvent(QResizeEvent*)
{
    QMutexLocker l(&mtx);
    init_image_nolock();
}

std::tuple<int, int> video_widget::preview_size() const
{
    QSize sz = size_.load(std::memory_order_acquire);
    return { sz.width(), sz.height() };
}

bool video_widget::fresh() const
{
    return fresh_.load(std::memory_order_acquire);
}

void video_widget::set_fresh(bool x)
{
    fresh_.store(x, std::memory_order_release);
}
