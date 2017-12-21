/* Copyright (c) 2012-2016 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "compat/util.hpp"
#include "spline-widget.hpp"
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QPen>
#include <QPixmap>
#include <QList>
#include <QPoint>
#include <QString>
#include <QRect>
#include <QApplication>
#include <QStyle>
#include <QMouseEvent>

#include <QDebug>

#include <cmath>
#include <algorithm>

spline_widget::spline_widget(QWidget *parent) :
    QWidget(parent),
    _config(nullptr),
    snap_x(0),
    snap_y(0),
    _x_step(10),
    _y_step(10),
    moving_control_point_idx(-1),
    _draw_function(true),
    _preview_only(false)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);
    setCursor(Qt::ArrowCursor);
}

spline_widget::~spline_widget()
{
    if (connection)
        QObject::disconnect(connection);
}

void spline_widget::setConfig(base_spline* spl)
{
    if (connection)
    {
        QObject::disconnect(connection);
        connection = QMetaObject::Connection();
    }

    _config = spl;

    if (spl)
    {
        update_range();

        std::shared_ptr<base_spline::base_settings> s = spl->get_settings();
        connection = connect(s.get(), &spline::base_settings::recomputed,
                             this, [this]() { reload_spline(); },
                             Qt::QueuedConnection);
    }
}

QColor spline_widget::colorBezier() const
{
    return spline_color;
}

void spline_widget::setColorBezier(QColor color)
{
    spline_color = color;
    repaint();
}

void spline_widget::force_redraw()
{
    _background = QPixmap();
    repaint();
}

void spline_widget::set_preview_only(bool val)
{
    _preview_only = val;
}

bool spline_widget::is_preview_only() const
{
    return _preview_only;
}

void spline_widget::drawBackground()
{
    _background = QPixmap(width(), height());

    QPainter painter(&_background);

    painter.fillRect(rect(), widget_bg_color);

    {
        QColor bg_color(112, 154, 209);
        if (!isEnabled() && !_preview_only)
            bg_color = QColor(176,176,180);
        painter.fillRect(pixel_bounds, bg_color);
    }

    QFont font;
    font.setPointSize(8);
    painter.setFont(font);
    QFontMetrics metrics(font);

    QColor color__(176, 190, 209, 127);

    if (!isEnabled())
        color__ = QColor(70, 90, 100, 96);

    const QPen pen(color__, 1, Qt::SolidLine, Qt::FlatCap);

    const int ystep = _y_step, xstep = _x_step;
    const qreal maxx = _config->max_input();
    const qreal maxy = _config->max_output();

    // horizontal grid
    for (int i = 0; i <= maxy; i += ystep)
    {
        const int y = int(pixel_bounds.height() - i * c.y() + pixel_bounds.y());
        drawLine(painter,
                 QPoint(pixel_bounds.x(), y),
                 QPoint(pixel_bounds.x() + pixel_bounds.width(), y),
                 pen);
        painter.drawText(QRectF(10,
                                y - metrics.height()/2.,
                                pixel_bounds.left(),
                                metrics.height()),
                         QString::number(i));
    }

    // vertical grid
    for (int i = 0; i <= maxx; i += xstep)
    {
        const int x = iround(pixel_bounds.x() + i * c.x());
        drawLine(painter,
                 QPoint(x, pixel_bounds.y()),
                 QPoint(x, pixel_bounds.y() + pixel_bounds.height()),
                 pen);
        const QString text = QString::number(i);
        painter.drawText(QRectF(x - metrics.width(text)/2.,
                                pixel_bounds.height() + 10 + metrics.height(),
                                metrics.width(text),
                                metrics.height()),
                         text);
    }
}

void spline_widget::drawFunction()
{
    _function = _background;
    QPainter painter(&_function);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const points_t points = _config->get_points();

    if (moving_control_point_idx >= 0 &&
        moving_control_point_idx < points.size())
    {
        const QPen pen(Qt::white, 1, Qt::SolidLine, Qt::FlatCap);
        const QPointF prev_ = point_to_pixel(QPointF(0, 0));
        QPoint prev(iround(prev_.x()), iround(prev_.y()));
        for (int i = 0; i < points.size(); i++)
        {
            const QPointF tmp_ = point_to_pixel(points[i]);
            const QPoint tmp(iround(tmp_.x()), iround(tmp_.y()));
            drawLine(painter, prev, tmp, pen);
            prev = tmp;
        }
    }

    const QColor color_ = progn(
        if (!isEnabled() && !_preview_only)
        {
            QColor color(spline_color);
            const int avg = int(float(color.red() + color.green() + color.blue())/3);
            return QColor(int(float(color.red() + avg)*.5f),
                          int(float(color.green() + avg)*.5f),
                          int(float(color.blue() + avg)*.5f),
                          96);
        }
        else
        {
            QColor color(spline_color);
            color.setAlphaF(color.alphaF()*.9);
            return color;
        }
    );

    painter.setPen(QPen(color_, 1.75, Qt::SolidLine, Qt::FlatCap));

//#define DEBUG_SPLINE
#ifndef DEBUG_SPLINE
    constexpr double step_ = 5;

    const double maxx = _config->max_input();
    const double step = step_ / c.x();

    QPainterPath path;

    path.moveTo(point_to_pixel(QPointF(0, 0)));

    const double max_x_pixel = point_to_pixel_(QPointF(maxx, 0)).x();

    auto check = [=](const QPointF& val) {
        return val.x() < max_x_pixel
               ? val
               : QPointF(max_x_pixel, val.y());
    };

    for (double k = 0; k < maxx; k += step*3)
    {
        const float next_1(_config->get_value_no_save(k + step*1));
        const float next_2(_config->get_value_no_save(k + step*2));
        const float next_3(_config->get_value_no_save(k + step*3));

        QPointF b(check(point_to_pixel_(QPointF(k + step*1, qreal(next_1))))),
                c(check(point_to_pixel_(QPointF(k + step*2, qreal(next_2))))),
                d(check(point_to_pixel_(QPointF(k + step*3, qreal(next_3)))));

        path.cubicTo(b, c, d);
    }

    painter.drawPath(path);
#else
    static constexpr int line_length_pixels = 3;
    const qreal max = _config->maxInput();
    const qreal step = clamp(line_length_pixels / c.x(), 5e-2, max);
    QPointF prev = point_to_pixel(QPoint(0, 0));
    for (qreal i = 0; i < max; i += step)
    {
        const qreal val = qreal(_config->get_value_no_save(i));
        const QPointF cur = point_to_pixel(QPointF(i, val));
        painter.drawLine(prev, cur);
        prev = cur;
    }
    {
        const qreal maxx = _config->maxInput();
        const qreal maxy = qreal(_config->get_value_no_save(maxx));
        painter.drawLine(QPointF(prev), point_to_pixel_(QPointF(maxx, maxy)));
    }
#endif

    const QRect r1(pixel_bounds.left(), 0, width() - pixel_bounds.left(), pixel_bounds.top()),
                r2(pixel_bounds.right(), 0, width() - pixel_bounds.right(), pixel_bounds.bottom());

    // prevent topward artifacts the lazy way
    painter.fillRect(r1, widget_bg_color);
    // same for rightward artifacts
    painter.fillRect(r2, widget_bg_color);

    const int alpha = !isEnabled() ? 64 : 120;
    if (!_preview_only)
    {
        for (int i = 0; i < points.size(); i++)
        {
            drawPoint(painter,
                      point_to_pixel(points[i]),
                      QColor(200, 200, 210, alpha),
                      isEnabled() ? QColor(50, 100, 120, 200) : QColor(200, 200, 200, 96));
        }
    }
}

void spline_widget::paintEvent(QPaintEvent *e)
{
    if (!_config)
        return;

    QPainter p(this);

    if (!_background.isNull())
    {
        if (_background.size() != size())
        {
            _background = QPixmap();
            _function = QPixmap();
        }
    }
    else
    {
        _draw_function = true;
        drawBackground();
    }

    if (_draw_function)
    {
        _draw_function = false;
        drawFunction();
    }

    p.drawPixmap(e->rect(), _function);

    // If the Tracker is active, the 'Last Point' it requested is recorded.
    // Show that point on the graph, with some lines to assist.
    // This new feature is very handy for tweaking the curves!
    QPointF last;
    if (_config->get_last_value(last) && isEnabled())
        drawPoint(p, point_to_pixel(last), QColor(255, 0, 0, 120));
}

void spline_widget::drawPoint(QPainter& painter, const QPointF& pos, const QColor& colBG, const QColor& border)
{
    painter.save();
    painter.setPen(QPen(border, 1, Qt::SolidLine, Qt::PenCapStyle::FlatCap));
    painter.setBrush(colBG);
    painter.drawEllipse(QRectF(pos.x() - point_size,
                                pos.y() - point_size,
                                point_size*2, point_size*2));
    painter.restore();
}

void spline_widget::drawLine(QPainter& painter, const QPoint& start, const QPoint& end, const QPen& pen)
{
    painter.save();
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(start, end);
    painter.restore();
}

void spline_widget::mousePressEvent(QMouseEvent *e)
{
    if (!_config || !isEnabled() || !is_in_bounds(e->pos()) || _preview_only)
    {
        clearFocus();
        return;
    }

    const int point_pixel_closeness_limit = get_closeness_limit();

    moving_control_point_idx = -1;

    points_t points = _config->get_points();

    if (e->button() == Qt::LeftButton)
    {
        bool is_touching_point = false;

        for (int i = 0; i < points.size(); i++)
        {
            if (point_within_pixel(points[i], e->pos()))
            {
                is_touching_point = true;
                moving_control_point_idx = i;
                break;
            }
        }

        if (!is_touching_point)
        {
            bool too_close = false;
            const QPoint pos = e->pos();

            for (int i = 0; i < points.size(); i++)
            {
                const QPointF pt = point_to_pixel(points[i]);
                const qreal x = std::fabs(pt.x() - pos.x());
                if (point_pixel_closeness_limit >= x)
                {
                    too_close = true;
                    break;
                }
            }

            if (!too_close)
            {
                _config->add_point(pixel_coord_to_point(e->pos()));
                show_tooltip(e->pos());
            }
        }

        _draw_function = true;
    }

    if (e->button() == Qt::RightButton)
    {
        if (_config)
        {
            int found_pt = -1;
            for (int i = 0; i < points.size(); i++)
            {
                if (point_within_pixel(points[i], e->pos()))
                {
                    found_pt = i;
                    break;
                }
            }

            if (found_pt != -1)
            {
                _config->remove_point(found_pt);
                _draw_function = true;
            }
        }
    }

    if (_draw_function)
        repaint();
}

void spline_widget::mouseMoveEvent(QMouseEvent *e)
{
    if (_preview_only && _config)
    {
        show_tooltip(e->pos());
        clearFocus();
        return;
    }

    if (!_config || !isEnabled() || !isActiveWindow() || (moving_control_point_idx != -1 && !hasFocus()))
    {
        clearFocus();
        return;
    }

    const int i = moving_control_point_idx;
    const points_t points = _config->get_points();
    const int sz = points.size();

    if (i >= 0 && i < sz)
    {
        const int point_closeness_limit = get_closeness_limit();
        bool overlap = false;

        const QPointF pix_ = point_to_pixel(pixel_coord_to_point(e->pos()));
        const QPoint pix(int(pix_.x()), int(pix_.y()));

        QPointF new_pt = pixel_coord_to_point(e->pos());

        if (i + 1 < points.size())
        {
            overlap |= points[i+1].x() - new_pt.x() < point_closeness_limit;
        }
        if (i > 0)
        {
            overlap |= new_pt.x() - points[i-1].x() < point_closeness_limit;
        }

        if (overlap)
            new_pt = QPointF(points[i].x(), new_pt.y());

        _config->move_point(i, new_pt);

        _draw_function = true;
        repaint();

        setCursor(Qt::ClosedHandCursor);
        show_tooltip(pix, new_pt);
    }
    else if (sz)
    {
        int i;
        bool is_on_point = is_on_pt(e->pos(), &i);

        if (is_on_point)
        {
            setCursor(Qt::CrossCursor);
            show_tooltip(e->pos(), points[i]);
        }
        else
        {
            setCursor(Qt::ArrowCursor);
            if (is_in_bounds(e->pos()))
                show_tooltip(e->pos());
            else
                QToolTip::hideText();
        }
    }
}

void spline_widget::mouseReleaseEvent(QMouseEvent *e)
{
    if (!_config || !isEnabled() || !isActiveWindow() || !hasFocus() || _preview_only)
    {
        clearFocus();
        return;
    }

    const bool redraw = moving_control_point_idx != -1;
    moving_control_point_idx = -1;

    if (e->button() == Qt::LeftButton)
    {
        {
            if (is_on_pt(e->pos(), nullptr))
                setCursor(Qt::CrossCursor);
            else
                setCursor(Qt::ArrowCursor);
        }

        if (is_in_bounds(e->pos()))
            show_tooltip(e->pos());
        else
            QToolTip::hideText();
    }

    if (redraw)
    {
        _draw_function = true;
        repaint();
    }
}

void spline_widget::reload_spline()
{
    // don't recompute here as the value's about to be recomputed in the callee
    update_range();
    update();
}

int spline_widget::get_closeness_limit()
{
    return iround(std::fmax(snap_x, 1));
}

void spline_widget::show_tooltip(const QPoint& pos, const QPointF& value_)
{
    const QPointF value = QPoint(0, 0) == value_ ? pixel_coord_to_point(pos) : value_;

    double x = value.x(), y = value.y();

    if (_preview_only)
        y = _config->get_value_no_save(x);

    const int x_ = iround(x), y_ = iround(y);

    using std::fabs;

    if (fabs(x_ - x) < 1e-3)
        x = x_;
    if (fabs(y_ - y) < 1e-3)
        y = y_;

    const bool is_fusion = QStringLiteral("fusion") == QApplication::style()->objectName();
    const int add_x = (is_fusion ? 25 : 0), add_y = (is_fusion ? 15 : 0);

    const QPoint pix(int(pos.x()) + add_x, int(pos.y()) + add_y);

    QToolTip::showText(mapToGlobal(pix),
                       QStringLiteral("value: %1x%2").arg(x, 0, 'f', 2).arg(y, 0, 'f', 2),
                       this,
                       rect(),
                       0);
}

bool spline_widget::is_in_bounds(const QPoint& pos) const
{
    constexpr int grace = point_size * 3;
    constexpr int bottom_grace = int(point_size * 1.5);
    return (pos.x() + grace        > pixel_bounds.left() &&
            pos.x() - grace        < pixel_bounds.right() &&
            pos.y() + grace        > pixel_bounds.top() &&
            pos.y() - bottom_grace < pixel_bounds.bottom());
}

void spline_widget::update_range()
{
    if (!_config)
        return;

    const int w = width(), h = height();
    const int mwl = 40, mhl = 20;
    const int mwr = 15, mhr = 35;

    pixel_bounds = QRect(mwl, mhl, (w - mwl - mwr), (h - mhl - mhr));
    c = QPointF(pixel_bounds.width() / _config->max_input(), pixel_bounds.height() / _config->max_output());
    _draw_function = true;

    _background = QPixmap();
    _function = QPixmap();

    repaint();
}

bool spline_widget::point_within_pixel(const QPointF& pt, const QPoint &pixel)
{
    const QPointF tmp = pixel - point_to_pixel(pt);
    return QPointF::dotProduct(tmp, tmp) < point_size * point_size;
}

void spline_widget::focusOutEvent(QFocusEvent* e)
{
    if (moving_control_point_idx != -1)
        QToolTip::hideText();
    moving_control_point_idx = -1;
    _draw_function = true;
    lower();
    setCursor(Qt::ArrowCursor);
    e->accept();
}

QPointF spline_widget::pixel_coord_to_point(const QPoint& point)
{
    qreal x = (point.x() - pixel_bounds.x()) / c.x();
    qreal y = (pixel_bounds.height() - point.y() + pixel_bounds.y()) / c.y();

    constexpr int c = 1000;

    if (snap_x > 0)
    {
        x += snap_x * .5;
        x -= std::fmod(x, snap_x);
        // truncate after few decimal places to reduce rounding errors.
        // round upward to nearest.
        x = int(x * c + .5/c) / double(c);
    }
    if (snap_y > 0)
    {
        y += snap_y * .5;
        y -= std::fmod(y, snap_y);
        // idem
        y = int(y * c + .5/c) / double(c);
    }

    if (x < 0)
        x = 0;
    if (x > _config->max_input())
        x = _config->max_input();

    if (y < 0)
        y = 0;
    if (y > _config->max_output())
        y = _config->max_output();

    return QPointF(x, y);
}

QPointF spline_widget::point_to_pixel_(const QPointF& point)
{
    return QPointF(pixel_bounds.x() + point.x() * c.x(),
                   pixel_bounds.y() + pixel_bounds.height() - point.y() * c.y());
}

QPoint spline_widget::point_to_pixel(const QPointF& point)
{
    QPointF pt(point_to_pixel_(point));

    return QPoint(iround(pt.x()), iround(pt.y()));
}

void spline_widget::resizeEvent(QResizeEvent *)
{
    update_range();
}

bool spline_widget::is_on_pt(const QPoint& pos, int* pt)
{
    if (!_config)
    {
        if (pt)
            *pt = -1;
        return false;
    }

    const points_t points = _config->get_points();

    for (int i = 0; i < points.size(); i++)
    {
        if (point_within_pixel(points[i], pos))
        {
            if (pt)
                *pt = i;
            return true;
        }
    }

    if (pt)
        *pt = -1;
    return false;
}
