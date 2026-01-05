#include "spline-widget.hpp"
#include "compat/math.hpp"

#include <algorithm>

#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QString>
#include <QToolTip>
#include <QtEvents>
#include <QPainterPath>

#include <QDebug>

#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
#   define OTR_OBSOLETE_QT_WORKAROUND
#endif

namespace spline_detail {

spline_widget::spline_widget(QWidget *parent) : QWidget(parent)
{
    setMouseTracking(true);
    //setFocusPolicy(Qt::ClickFocus);
    setCursor(Qt::ArrowCursor);
}

spline_widget::~spline_widget()
{
    if (connection)
    {
        QObject::disconnect(connection);
        connection = {};
    }
}

void spline_widget::set_config(base_spline* spl)
{
    if (connection)
    {
        QObject::disconnect(connection);
        connection = {};
    }

    config = spl;

    if (spl)
    {
        std::shared_ptr<base_settings> s = spl->get_settings();
        connection = connect(&*s, &base_settings::recomputed,
                             this, [this] { reload_spline(); },
                             Qt::QueuedConnection);
    }

    reload_spline();
}

QColor spline_widget::colorBezier() const
{
    return spline_color;
}

void spline_widget::setColorBezier(QColor const& color)
{
    spline_color = color;
    repaint();
}

void spline_widget::force_redraw()
{
    background_img = {};
    repaint();
}

void spline_widget::set_preview_only(bool val)
{
    preview_only = val;
}

bool spline_widget::is_preview_only() const
{
    return preview_only;
}

void spline_widget::drawBackground()
{
    QPainter painter(&background_img);

    painter.fillRect(rect(), widget_bg_color);

    {
        QColor bg_color(112, 154, 209);
        if (!isEnabled() && !preview_only)
            bg_color = QColor(176,176,180);
        painter.fillRect(pixel_bounds, bg_color);
    }

    QFont font;
    font.setPointSize(8);
    font.setStyleHint(QFont::Monospace, QFont::PreferAntialias);
    painter.setFont(font);
    const QFontMetricsF metrics(font);

    QColor color__(176, 190, 209, 127);

    if (!isEnabled())
        color__ = QColor(70, 90, 100, 96);

    const QPen pen(color__, 1, Qt::SolidLine, Qt::FlatCap);

    const int ystep = (int)std::ceil(y_step_), xstep = (int)std::ceil(x_step_);
    const double maxx = config->max_input();
    const double maxy = config->max_output();
#ifndef OTR_OBSOLETE_QT_WORKAROUND
    double space_width = metrics.horizontalAdvance(' ');
#else
    double space_width = metrics.boundingRect(' ').right();
#endif
    painter.setPen(palette().text().color());

    // vertical grid
    for (int i = 0; i <= maxy; i += ystep)
    {
        const double y = pixel_bounds.height() - i * c.y() + pixel_bounds.y();
        drawLine(painter,
                 QPointF(pixel_bounds.x(), y),
                 QPointF(pixel_bounds.x() + pixel_bounds.width(), y),
                 pen);
        QString text = QString::number(i);
        QRectF rect = metrics.boundingRect(text);
#ifndef OTR_OBSOLETE_QT_WORKAROUND
        double advance = metrics.horizontalAdvance(text);
#else
        double advance = rect.right();
#endif
        painter.drawText(QPointF(pixel_bounds.x() - advance - space_width,
                                 y - rect.height()/2 - rect.top()),
                         text);
    }

    // horizontal grid
    for (int i = 0; i <= maxx; i += xstep)
    {
        const double x = pixel_bounds.x() + i * c.x();
        drawLine(painter,
                 QPointF(x, pixel_bounds.y()),
                 QPointF(x, pixel_bounds.y() + pixel_bounds.height()),
                 pen);

        const QString text = QString::number(i);
        QRectF rect = metrics.boundingRect(text);
#ifndef OTR_OBSOLETE_QT_WORKAROUND
        double advance = metrics.horizontalAdvance(text);
#else
        double advance = metrics.boundingRect(text).right();
#endif
        painter.drawText(QPointF(x - advance/2 - rect.left(),
                                 pixel_bounds.bottom() + metrics.lineSpacing()),
                         text);
    }
}

void spline_widget::drawFunction()
{
    QPainter painter(&spline_img);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const points_t& points = config->get_points();

    if (moving_control_point_idx >= 0 &&
        moving_control_point_idx < points.size())
    {
        const QPen pen(Qt::white, 1, Qt::SolidLine, Qt::FlatCap);
        QPointF prev = point_to_pixel({});
        for (const auto& point : points)
        {
            const QPointF tmp = point_to_pixel(point);
            drawLine(painter, prev, tmp, pen);
            prev = tmp;
        }
    }

    const QColor color_ = progn(
        if (!isEnabled() && !preview_only)
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

    painter.setPen(QPen(color_, 2, Qt::SolidLine, Qt::FlatCap));

    const double dpr = screen_dpi();
    const double line_length_pixels = std::fmax(1, dpr);
    const double step = std::fmax(.1, line_length_pixels / c.x());
    const double maxx = config->max_input();

#define USE_CUBIC_SPLINE
#if defined USE_CUBIC_SPLINE
    QPainterPath path;

    path.moveTo(point_to_pixel({}));

    const double max_x_pixel = point_to_pixel({maxx, 0}).x();

    auto clamp = [=](const QPointF& val) {
        return val.x() <= max_x_pixel
               ? val
               : QPointF{max_x_pixel, val.y()};
    };

    const auto fn = [&] (double k) {
      const auto next_1 = config->get_value_no_save(k + step*1);
      const auto next_2 = config->get_value_no_save(k + step*2);
      const auto next_3 = config->get_value_no_save(k + step*3);

      QPointF b(clamp(point_to_pixel({k + step*1, next_1}))),
          c(clamp(point_to_pixel({k + step*2, next_2}))),
          d(clamp(point_to_pixel({k + step*3, next_3})));

      path.cubicTo(b, c, d);
    };

    for (double k = 0; k < maxx; k += step*3) // NOLINT
        fn(k);

    fn(maxx);

    painter.drawPath(path);
#else
    QPointF prev = point_to_pixel({});
    for (double i = 0; i < maxx; i += step) // NOLINT
    {
        const auto val = config->get_value_no_save(i);
        const QPointF cur = point_to_pixel({i, val});
        painter.drawLine(prev, cur);
        prev = cur;
    }
    {
        const double maxx = config->max_input();
        const double maxy = config->get_value_no_save(maxx);
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
    if (!preview_only)
    {
        for (auto const& point : points)
        {
            drawPoint(painter,
                      point_to_pixel(point),
                      QColor(200, 200, 210, alpha),
                      isEnabled() ? QColor(50, 100, 120, 200) : QColor(200, 200, 200, 96));
        }
    }
}

QSize spline_widget::minimumSizeHint() const
{
    const double dpi = screen_dpi();
    return { iround(800 * dpi), iround(250 * dpi) };
}

void spline_widget::paintEvent(QPaintEvent *e)
{
    if (!config)
        return;

    QPainter p(this);

    const double dpr = devicePixelRatioF();
    const int W = iround(width() * dpr);
    const int H = iround(height() * dpr);

    if (background_img.size() != QSize(W, H))
    {
        update_range();

        background_img = { W, H };
        background_img.setDevicePixelRatio(dpr);
        drawBackground();
        draw_function = true;
    }

    if (draw_function)
    {
        draw_function = false;
        spline_img = background_img;
        drawFunction();
    }

    p.drawPixmap(e->rect(), spline_img);

    // If the Tracker is active, the 'Last Point' it requested is recorded.
    // Show that point on the graph, with some lines to assist.
    // This new feature is very handy for tweaking the curves!
    QPointF last;
    if (config->get_last_value(last) && isEnabled())
        drawPoint(p, point_to_pixel(last), QColor(255, 0, 0, 120));
}

void spline_widget::drawPoint(QPainter& painter, const QPointF& pos, const QColor& colBG, const QColor& border)
{
    painter.save();
    painter.setPen(QPen(border, 1, Qt::SolidLine, Qt::PenCapStyle::FlatCap));
    painter.setBrush(colBG);
    painter.drawEllipse(QRectF{pos.x() - point_size_in_pixels,
                               pos.y() - point_size_in_pixels,
                               point_size_in_pixels*2, point_size_in_pixels*2});
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
    if (!config || !isEnabled() || !is_in_bounds(e->position()) || preview_only)
        return;

    const double min_dist = min_pt_distance();

    moving_control_point_idx = -1;

    const points_t& points = config->get_points();

    if (e->button() == Qt::LeftButton)
    {
        bool is_touching_point = false;

        for (int i = 0; i < points.size(); i++)
        {
            if (point_within_pixel(points[i], e->position()))
            {
                is_touching_point = true;
                moving_control_point_idx = i;
                break;
            }
        }

        if (!is_touching_point)
        {
            bool too_close = false;
            const QPointF pos = pixel_to_point(e->position());

            for (QPointF const& point : points)
            {
                const double x = std::fabs(point.x() - pos.x());
                if (min_dist > x)
                {
                    too_close = true;
                    break;
                }
            }

            if (!too_close)
            {
                config->add_point(pixel_to_point(e->position()));
                show_tooltip(e->pos());
            }
        }

        draw_function = true;
    }

    if (e->button() == Qt::RightButton)
    {
        if (config)
        {
            for (int i = 0; i < points.size(); i++)
            {
                if (point_within_pixel(points[i], e->position()))
                {
                    config->remove_point(i);
                    draw_function = true;
                    break;
                }
            }
        }
    }

    if (draw_function)
        repaint();
}

void spline_widget::mouseMoveEvent(QMouseEvent *e)
{
    if (preview_only && config)
    {
        show_tooltip(e->pos());
        return;
    }

    if (!config || !isEnabled() || !isActiveWindow())
    {
        QToolTip::hideText();
        return;
    }

    const int i = moving_control_point_idx;
    const points_t& points = config->get_points();
    const int sz = points.size();

    if (i >= 0 && i < sz)
    {
        const double min_dist = min_pt_distance();
        QPointF new_pt = pixel_to_point(e->position());

        const bool has_prev = i > 0, has_next = i + 1 < points.size();

        auto check_next = [&] {
            return points[i+1].x() - new_pt.x() >= min_dist;
        };

        auto check_prev = [&] {
            return new_pt.x() - points[i-1].x() >= min_dist;
        };

        if (has_prev && !check_prev())
        {
            new_pt.rx() = points[i-1].x() + min_dist + 1e-4;
        }

        if (has_next && !check_next())
        {
            new_pt.rx() = points[i+1].x() - min_dist - 1e-4;
        }

        setCursor(Qt::ClosedHandCursor);
        show_tooltip(point_to_pixel(new_pt).toPoint(), new_pt);

        if ((!has_prev || check_prev()) && (!has_next || check_next()))
        {
            config->move_point(i, new_pt);
            draw_function = true;
            repaint();
        }
    }
    else if (sz)
    {
        int i;
        bool is_on_point = is_on_pt(e->position(), &i);

        if (is_on_point)
        {
            setCursor(Qt::CrossCursor);
            show_tooltip(e->pos(), points[i]);
        }
        else
        {
            setCursor(Qt::ArrowCursor);
            if (is_in_bounds(e->position()))
                show_tooltip(e->pos());
            else
                QToolTip::hideText();
        }
    }
}

void spline_widget::mouseReleaseEvent(QMouseEvent *e)
{
    if (!config || !isEnabled() || !isActiveWindow() || preview_only)
        return;

    const bool redraw = moving_control_point_idx != -1;
    moving_control_point_idx = -1;

    if (e->button() == Qt::LeftButton)
    {
        {
            if (is_on_pt(e->position(), nullptr))
                setCursor(Qt::CrossCursor);
            else
                setCursor(Qt::ArrowCursor);
        }

        if (is_in_bounds(e->position()))
            show_tooltip(e->pos());
        else
            QToolTip::hideText();
    }

    if (redraw)
    {
        draw_function = true;
        repaint();
    }
}

void spline_widget::reload_spline()
{
    // don't recompute here as the value's about to be recomputed in the callee
    background_img = {};
    update();
}

double spline_widget::min_pt_distance() const
{
    double pt = 3*point_size_in_pixels / c.x();
    pt = snap(pt, snap_x);
    return pt;
}

void spline_widget::show_tooltip(const QPoint& pos, const QPointF& value_)
{
    const QPointF value = value_.isNull() ? pixel_to_point(pos) : value_;

    double x = value.x(), y = value.y();

    if (preview_only)
        y = config->get_value_no_save(x);

    const int x_ = iround(x), y_ = iround(y);

    if (std::fabs(x_ - x) < 1e-3)
        x = x_;
    if (std::fabs(y_ - y) < 1e-3)
        y = y_;

    // the native OSX style doesn't look right otherwise
#if defined __APPLE__
    constexpr int off_x = 0, off_y = 0;
#else
    constexpr int off_x = 25, off_y = 15;
#endif

    const QPoint pix(pos.x() + off_x, pos.y() + off_y);

    QToolTip::showText(mapToGlobal(pix),
                       QString{"value: %1x%2"}.arg(x, 0, 'f', 2).arg(y, 0, 'f', 2),
                       this,
                       rect(),
                       0);
}

bool spline_widget::is_in_bounds(const QPointF& pos) const
{
    const int grace = (int)std::ceil(point_size_in_pixels * 3);
    const int bottom_grace = (int)std::ceil(point_size_in_pixels * 1.5);
    return (pos.x() + grace        > pixel_bounds.left() &&
            pos.x() - grace        < pixel_bounds.right() &&
            pos.y() + grace        > pixel_bounds.top() &&
            pos.y() - bottom_grace < pixel_bounds.bottom());
}

void spline_widget::update_range()
{
    if (!config)
        return;

    const int w = width(), h = height();
    const int mwl = 40, mhl = 20;
    const int mwr = 15, mhr = 35;

    pixel_bounds = { mwl, mhl, (w - mwl - mwr), (h - mhl - mhr) };
    c = {
        pixel_bounds.width() / std::fmax(1, config->max_input()),
        pixel_bounds.height() / std::fmax(1, config->max_output())
    };

    draw_function = true;
    background_img = {};
    spline_img = {};
}

bool spline_widget::point_within_pixel(const QPointF& pt, const QPointF& pixel)
{
    const QPointF tmp = pixel - point_to_pixel(pt);
    return QPointF::dotProduct(tmp, tmp) < point_size_in_pixels * point_size_in_pixels;
}

void spline_widget::focusOutEvent(QFocusEvent* e)
{
    if (moving_control_point_idx != -1)
        QToolTip::hideText();
    moving_control_point_idx = -1;
    draw_function = true;
    lower();
    setCursor(Qt::ArrowCursor);
    e->accept();
}

double spline_widget::snap(double x, double snap_value)
{
    if (snap_value > 0)
    {
        constexpr int c = 1000;
        x += snap_value * .5;
        x -= std::fmod(x, snap_value);
        // truncate after few decimal places to reduce rounding errors.
        // round upward.
        x = int(x * c + .5/c) / double(c);
    }

    return x;
}

QPointF spline_widget::pixel_to_point(const QPointF& point)
{
    double x = (point.x() - pixel_bounds.x()) / c.x();
    double y = (pixel_bounds.height() - point.y() + pixel_bounds.y()) / c.y();

    if (snap_x > 0)
        x = snap(x, snap_x);
    if (snap_y > 0)
        y = snap(y, snap_y);

    x = std::clamp(x, 0., config->max_input());
    y = std::clamp(y, 0., config->max_output());

    return { x, y };
}

QPointF spline_widget::point_to_pixel(const QPointF& point)
{
    return {
        pixel_bounds.x() + point.x() * c.x(),
        pixel_bounds.y() + pixel_bounds.height() - point.y() * c.y()
    };
}

void spline_widget::resizeEvent(QResizeEvent *)
{
    reload_spline();
}

bool spline_widget::is_on_pt(const QPointF& pos, int* pt)
{
    if (!config)
    {
        if (pt)
            *pt = -1;
        return false;
    }

    const points_t& points = config->get_points();

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

void spline_widget::changeEvent(QEvent* e)
{
    switch (e->type())
    {
    case QEvent::EnabledChange:
        background_img = {}; spline_img = {};
        update();
        break;
    default:
        break;
    }
}
} // ns spline_detail
