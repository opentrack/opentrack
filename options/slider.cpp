/* Copyright (c) 2015-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "slider.hpp"
#include <cmath>

namespace options {

slider_value::slider_value(double cur, double min, double max) :
    cur_(cur),
    min_(min),
    max_(max)
{
    if (min_ > max_)
        min_ = max_;
    if (cur_ > max_)
        cur_ = max;
    if (cur_ < min_)
        cur_ = min_;
}

bool slider_value::operator==(const slider_value& v) const
{
    constexpr double eps = 2e-3;

#if 1
    return (std::fabs(v.cur_ - cur_) < eps &&
            std::fabs(v.min_ - min_) < eps &&
            std::fabs(v.max_ - max_) < eps);
#else
    return (std::fabs(v.cur_ - cur_) < eps);
#endif
}

bool slider_value::operator!=(const slider_value& v) const
{
    return !(*this == v);
}

slider_value slider_value::update_from_slider(int pos, int q_min, int q_max) const
{
    slider_value v(*this);

    const int q_diff = q_max - q_min;
    const double sv_pos = q_diff == 0
                          ? 0
                          : (((pos - q_min) * (v.max() - v.min())) / q_diff + v.min());

    if (sv_pos < v.min())
        v = { v.min(), v.min(), v.max() };
    else if (sv_pos > v.max())
        v = { v.max(), v.min(), v.max() };
    else
        v = { sv_pos, v.min(), v.max() };
    return v;
}

int slider_value::to_slider_pos(int q_min, int q_max) const
{
    const int q_diff = q_max - q_min;
    const double div = max() - min();

    if (std::fabs(div) < 1e-4)
        return q_min;
    else
        return int(std::round(((cur() - min()) * q_diff / div) + q_min));
}

} // ns options

using options::slider_value;

QDataStream& operator << (QDataStream& out, const slider_value& v)
{
    out << v.cur()
        << v.min()
        << v.max();
    return out;
}

QDebug operator << (QDebug dbg, const slider_value& v)
{
    return dbg << QStringLiteral("slider_value(cur=%1, min=%2, max=%3)")
                    .arg(v.cur()).arg(v.min()).arg(v.max()).toUtf8().constData();
}

QDataStream& operator >> (QDataStream& in, slider_value& v)
{
    double cur = 0, min = 0, max = 0;
    in >> cur >> min >> max;
    v = slider_value(cur, min, max);
    return in;
}
