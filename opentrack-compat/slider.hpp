/* Copyright (c) 2016 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "export.hpp"
#include <QMetaType>
#include <QDataStream>
#include <QDebug>
#include <cmath>

namespace options
{
    class OPENTRACK_COMPAT_EXPORT slider_value final
    {
        double cur_, min_, max_;
    public:
        slider_value(double cur, double min, double max);
        slider_value(const slider_value& v);
        slider_value();
        slider_value& operator=(const slider_value& v);
        bool operator==(const slider_value& v) const;
        operator double() const { return cur_; }
        double cur() const { return cur_; }
        double min() const { return min_; }
        double max() const { return max_; }
        slider_value update_from_slider(int pos, int q_min, int q_max) const;
        int to_slider_pos(int q_min, int q_max) const;
    };
}

QT_BEGIN_NAMESPACE

inline QDebug operator << (QDebug dbg, const options::slider_value& val)
{
    return dbg << val.cur();
}

inline QDataStream& operator << (QDataStream& out, const options::slider_value& v)
{
    out << v.cur()
        << v.min()
        << v.max();
    return out;
}

inline QDataStream& operator >> (QDataStream& in, options::slider_value& v)
{
    double cur, min, max;
    in >> cur;
    in >> min;
    in >> max;
    v = options::slider_value(cur, min, max);
    return in;
}

QT_END_NAMESPACE

Q_DECLARE_METATYPE(options::slider_value)
