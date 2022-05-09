#pragma once

#include "cv/numeric.hpp"
#include "compat/timer.hpp"
#include "pt-settings.hpp"
#include <optional>
#include <array>

namespace pt_point_filter_impl {

using namespace numeric_types;
using PointOrder = std::array<numeric_types::vec2, 3>;

class point_filter final
{
    PointOrder state_;
    std::optional<Timer> t;
    const pt_settings& s;

public:
    void reset();
    const PointOrder& operator()(const PointOrder& input, f deadzone_amount);

    explicit point_filter(const pt_settings& s);
    ~point_filter() = default;

    OTR_DISABLE_MOVE_COPY(point_filter);
};

} // ns pt_point_filter_impl

using point_filter = pt_point_filter_impl::point_filter;
