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

slider_value::slider_value(const slider_value& v) : slider_value(v.cur(), v.min(), v.max())
{
}

slider_value::slider_value() : slider_value(0, 0, 0)
{
}

slider_value& slider_value::operator=(const slider_value& v)
{
    cur_ = v.cur_;

    min_ = v.min_;
    max_ = v.max_;

    return *this;
}

bool slider_value::operator==(const slider_value& v) const
{
    using std::fabs;

    constexpr double eps = 2e-3;

#if 1
    return (fabs(v.cur_ - cur_) < eps &&
            fabs(v.min_ - min_) < eps &&
            fabs(v.max_ - max_) < eps);
#else
    return (fabs(v.cur_ - cur_) < eps);
#endif
}

slider_value slider_value::update_from_slider(int pos, int q_min, int q_max) const
{
    slider_value v(*this);

    const int q_diff = q_max - q_min;
    const double sv_pos = q_diff == 0
                          ? 0
                          : (((pos - q_min) * (v.max() - v.min())) / q_diff + v.min());

    if (sv_pos < v.min())
        v = slider_value(v.min(), v.min(), v.max());
    else if (sv_pos > v.max())
        v = slider_value(v.max(), v.min(), v.max());
    else
        v = slider_value(sv_pos, v.min(), v.max());
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

} // end ns options

QT_BEGIN_NAMESPACE

QDataStream& operator << (QDataStream& out, const options::slider_value& v)
{
    out << v.cur()
        << v.min()
        << v.max();
    return out;
}

QDebug operator << (QDebug dbg, const options::slider_value& val)
{
    return dbg << val.cur();
}

QDataStream& operator >> (QDataStream& in, options::slider_value& v)
{
    double cur = 0, min = 0, max = 0;
    in >> cur >> min >> max;
    v = options::slider_value(cur, min, max);
    return in;
}

QT_END_NAMESPACE
