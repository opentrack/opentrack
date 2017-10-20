/* Copyright (c) 2016 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "export.hpp"

#include "compat/util.hpp"

#include <QMetaType>
#include <QDataStream>
#include <QDebug>

namespace options
{
    class OTR_OPTIONS_EXPORT slider_value final
    {
        double cur_, min_, max_;

        template<typename t>
        using arith_conversion_t = std::enable_if_t<std::is_arithmetic_v<std::decay_t<t>>, std::decay_t<t>>;
    public:
        slider_value(double cur, double min, double max);

        template<typename t, typename u, typename v> slider_value(t cur, u min, v max) :
            cur_(double(cur)),
            min_(double(min)),
            max_(double(max))
        {}

        template<typename t>
        never_inline
        explicit operator arith_conversion_t<t>() const
        {
            return t(cur_);
        }

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

QDebug operator << (QDebug dbg, const options::slider_value& val);
QDataStream& operator << (QDataStream& out, const options::slider_value& v);
QDataStream& operator >> (QDataStream& in, options::slider_value& v);

QT_END_NAMESPACE

