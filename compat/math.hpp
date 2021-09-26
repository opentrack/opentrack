#pragma once

#include "macros.hpp"

#include <cmath>
#include <type_traits>

template<typename t>
inline auto iround(t val) -> std::enable_if_t<std::is_floating_point_v<remove_cvref_t<t>>, int>
{
    return (int)std::round(val);
}

template <typename t>
force_inline
constexpr int signum(const t& x)
{
    return x < t{0} ? -1 : 1;
}
