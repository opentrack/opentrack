/* Copyright (c) 2012-2016 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "spline-widget.hpp"
#include "compat/math.hpp"
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QToolTip>
#include <QApplication>
#include <QtEvents>

#include <QDebug>

#include <cmath>
#include <algorithm>

spline_widget::spline_widget(QWidget *parent) : QWidget(parent)
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
                             this, [this] { reload_spline(); },
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
    QFontMetricsF metrics(font);

    QColor color__(176, 190, 209, 127);

    if (!isEnabled())
        color__ = QColor(70, 90, 100, 96);

    const QPen pen(color__, 1, Qt::SolidLine, Qt::FlatCap);

    const int ystep = _y_step, xstep = _x_step;
    const double maxx = _config->max_input();
    const double maxy = _config->max_output();

    // horizontal grid
    for (int i = 0; i <= maxy; i += ystep)
    {
        const double y = pixel_bounds.height() - i * c.y() + pixel_bounds.y();
        drawLine(painter,
                 QPointF(pixel_bounds.x(), y),
                 QPointF(pixel_bounds.x() + pixel_bounds.width(), y),
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
        const double x = pixel_bounds.x() + i * c.x();
        drawLine(painter,
                 QPointF(x, pixel_bounds.y()),
                 QPointF(x, pixel_bounds.y() + pixel_bounds.height()),
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
    QPainter painter(&_function);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const points_t points = _config->get_points();

    if (moving_control_point_idx >= 0 &&
        moving_control_point_idx < points.size())
    {
        const QPen pen(Qt::white, 1, Qt::SolidLine, Qt::FlatCap);
        const QPointF prev_ = point_to_pixel({});
        QPointF prev(iround(prev_.x()), iround(prev_.y()));
        for (int i = 0; i < points.size(); i++)
        {
            const QPointF tmp = point_to_pixel(points[i]);
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
    const double step = std::fmax(1e-4, step_ / c.x());

    QPainterPath path;

    path.moveTo(point_to_pixel({}));

    const double max_x_pixel = point_to_pixel({maxx, 0}).x();

    auto clamp = [=](const QPointF& val) {
        return val.x() <= max_x_pixel
               ? val
               : QPointF{max_x_pixel, val.y()};
    };

    for (double k = 0; k < maxx; k += step*3)
    {
        const auto next_1 = (double) _config->get_value_no_save(k + step*1);
        const auto next_2 = (double) _config->get_value_no_save(k + step*2);
        const auto next_3 = (double) _config->get_value_no_save(k + step*3);

        QPointF b(clamp(point_to_pixel({k + step*1, next_1}))),
                c(clamp(point_to_pixel({k + step*2, next_2}))),
                d(clamp(point_to_pixel({k + step*3, next_3})));

        path.cubicTo(b, c, d);
    }

    painter.drawPath(path);
#else
    constexpr int line_length_pixels = 3;
    const double max = _config->max_input();
    const double step = clamp(line_length_pixels / c.x(), 5e-2, max);
    QPointF prev = point_to_pixel({});
    for (double i = 0; i < max; i += step)
    {
        const auto val = (double) _config->get_value_no_save(i);
        const QPointF cur = point_to_pixel({i, val});
        painter.drawLine(prev, cur);
        prev = cur;
    }
    {
        const double maxx = _config->max_input();
        const double maxy = double(_config->get_value_no_save(maxx));
        painter.drawLine(prev, point_to_pixel({ maxx, maxy }));
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

    const double dpr = devicePixelRatioF();
    const int W = iround(width() * dpr);
    const int H = iround(height() * dpr);

    if (_background.size() != QSize(W, H))
    {
        _background = QPixmap(W, H);
        _background.setDevicePixelRatio(dpr);
        _draw_function = true;
        drawBackground();
    }

    if (_draw_function)
    {
        _draw_function = false;
        _function = _background;
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

void spline_widget::drawLine(QPainter& painter, const QPointF& start, const QPointF& end, const QPen& pen)
{
    painter.save();
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(start, end);
    painter.restore();
}

void spline_widget::mousePressEvent(QMouseEvent *e)
{
    if (!_config || !isEnabled() || !is_in_bounds(e->localPos()) || _preview_only)
    {
        clearFocus();
        return;
    }

    const double point_pixel_closeness_limit = get_closeness_limit();

    moving_control_point_idx = -1;

    points_t points = _config->get_points();

    if (e->button() == Qt::LeftButton)
    {
        bool is_touching_point = false;

        for (int i = 0; i < points.size(); i++)
        {
            if (point_within_pixel(points[i], e->localPos()))
            {
                is_touching_point = true;
                moving_control_point_idx = i;
                break;
            }
        }

        if (!is_touching_point)
        {
            bool too_close = false;
            const QPointF pos = e->localPos();

            for (int i = 0; i < points.size(); i++)
            {
                const QPointF pt = point_to_pixel(points[i]);
                const double x = std::fabs(pt.x() - pos.x());
                if (point_pixel_closeness_limit >= x)
                {
                    too_close = true;
                    break;
                }
            }

            if (!too_close)
            {
                _config->add_point(pixel_to_point(e->localPos()));
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
                if (point_within_pixel(points[i], e->localPos()))
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
        const double point_closeness_limit = get_closeness_limit();
        QPointF new_pt = pixel_to_point(e->localPos());
        const QPointF pix = point_to_pixel(new_pt);
        //bool overlap = false;
        const bool has_prev = i > 0, has_next = i + 1 < points.size();

        auto check_next = [&] {
            return !has_next || points[i+1].x() - new_pt.x() < point_closeness_limit;
        };

        auto check_prev = [&] {
            return !has_prev || new_pt.x() - points[i-1].x() < point_closeness_limit;
        };

        const bool status_next = !has_next || !check_next();
        const bool status_prev = !has_prev || !check_prev();

        if (!status_prev)
            new_pt = { points[i-1].x() + point_closeness_limit, new_pt.y() };

        if (!status_next)
            new_pt = { points[i+1].x() - point_closeness_limit, new_pt.y() };

        if (check_prev() && check_next())
            new_pt = { points[i].x(), new_pt.y() };

        _config->move_point(i, new_pt);

        _draw_function = true;
        repaint();

        setCursor(Qt::ClosedHandCursor);
        show_tooltip(pix.toPoint(), new_pt);
    }
    else if (sz)
    {
        int i;
        bool is_on_point = is_on_pt(e->localPos(), &i);

        if (is_on_point)
        {
            setCursor(Qt::CrossCursor);
            show_tooltip(e->pos(), points[i]);
        }
        else
        {
            setCursor(Qt::ArrowCursor);
            if (is_in_bounds(e->localPos()))
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
            if (is_on_pt(e->localPos(), nullptr))
                setCursor(Qt::CrossCursor);
            else
                setCursor(Qt::ArrowCursor);
        }

        if (is_in_bounds(e->localPos()))
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

double spline_widget::get_closeness_limit()
{
    return std::fmax(snap_x, 1);
}

void spline_widget::show_tooltip(const QPoint& pos, const QPointF& value_)
{
    const QPointF value = value_.isNull() ? pixel_to_point(pos) : value_;

    double x = value.x(), y = value.y();

    if (_preview_only)
        y = _config->get_value_no_save(x);

    const int x_ = iround(x), y_ = iround(y);

    if (std::fabs(x_ - x) < 1e-3)
        x = x_;
    if (std::fabs(y_ - y) < 1e-3)
        y = y_;

    static const bool is_fusion = QStringLiteral("fusion") == QApplication::style()->objectName();
    // no fusion means OSX
    const int add_x = (is_fusion ? 25 : 0), add_y = (is_fusion ? 15 : 0);

    const QPoint pix(pos.x() + add_x, pos.y() + add_y);

    QToolTip::showText(mapToGlobal(pix),
                       QString{"value: %1x%2"}.arg(x, 0, 'f', 2).arg(y, 0, 'f', 2),
                       this,
                       rect(),
                       0);
}

bool spline_widget::is_in_bounds(const QPointF& pos) const
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
    c = { pixel_bounds.width() / _config->max_input(), pixel_bounds.height() / _config->max_output() };
    _draw_function = true;

    _background = QPixmap();
    _function = QPixmap();

    repaint();
}

bool spline_widget::point_within_pixel(const QPointF& pt, const QPointF& pixel)
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

QPointF spline_widget::pixel_to_point(const QPointF& point)
{
    double x = (point.x() - pixel_bounds.x()) / c.x();
    double y = (pixel_bounds.height() - point.y() + pixel_bounds.y()) / c.y();

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

    x = clamp(x, 0, _config->max_input());
    y = clamp(y, 0, _config->max_output());

    return { x, y };
}

QPointF spline_widget::point_to_pixel(const QPointF& point)
{
    return QPointF(pixel_bounds.x() + point.x() * c.x(),
                   pixel_bounds.y() + pixel_bounds.height() - point.y() * c.y());
}

void spline_widget::resizeEvent(QResizeEvent *)
{
    update_range();
}

bool spline_widget::is_on_pt(const QPointF& pos, int* pt)
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
