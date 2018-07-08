/* Copyright (c) 2016 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "export.hpp"
#include "compat/macros.hpp"

#include <type_traits>

#include <QDataStream>
#include <QMetaType>
#include <QDebug>

namespace options
{
    class OTR_OPTIONS_EXPORT slider_value final
    {
        double cur_, min_, max_;

    public:
        constexpr slider_value(double cur, double min, double max);

        template<typename t, typename u, typename v>
        constexpr slider_value(t cur, u min, v max) :
            cur_(double(cur)),
            min_(double(min)),
            max_(double(max))
        {}

        slider_value& operator=(const slider_value& v) = default;
        constexpr slider_value(const slider_value& v) = default;
        constexpr slider_value() : slider_value{0, 0, 0} {};

        constexpr bool operator==(const slider_value& v) const;
        constexpr bool operator!=(const slider_value& v) const;
        constexpr operator double() const { return cur_; }
        constexpr double cur() const { return cur_; }
        constexpr double min() const { return min_; }
        constexpr double max() const { return max_; }
        constexpr slider_value update_from_slider(int pos, int q_min, int q_max) const;
        int to_slider_pos(int q_min, int q_max) const;
    };
}

OTR_OPTIONS_EXPORT QDebug operator << (QDebug dbg, const options::slider_value& val);
OTR_OPTIONS_EXPORT QDataStream& operator << (QDataStream& out, const options::slider_value& v);
OTR_OPTIONS_EXPORT QDataStream& operator >> (QDataStream& in, options::slider_value& v);
