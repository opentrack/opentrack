#pragma once

#include "macros.hpp"

#include <cmath>
#include <type_traits>

namespace util_detail {

template<typename n>
inline auto clamp_float(n val, n min_, n max_)
{
    return std::fmin(std::fmax(val, min_), max_);
}

template<typename t>
struct clamp final
{
    static inline auto clamp_(t val, t min_, t max_)
    {
        if (unlikely(val > max_))
            return max_;
        if (unlikely(val < min_))
            return min_;
        return val;
    }
};

template<>
struct clamp<float>
{
    static inline auto clamp_(float val, float min_, float max_)
    {
        return clamp_float(val, min_, max_);
    }
};

template<>
struct clamp<double>
{
    static inline auto clamp_(double val, double min_, double max_)
    {
        return clamp_float(val, min_, max_);
    }
};

} // ns util_detail

template<typename t, typename u, typename v>
inline auto clamp(const t& val, const u& min, const v& max)
{
    using w = cv_qualified<decltype(val + min + max)>;
    return ::util_detail::clamp<w>::clamp_(val, min, max);
}

template<typename t>
inline auto iround(t val) -> std::enable_if_t<std::is_floating_point_v<remove_cvref_t<t>>, int>
{
    return (int)std::round(val);
}

template<typename t>
inline auto uround(t val) -> std::enable_if_t<std::is_floating_point_v<remove_cvref_t<t>>, unsigned>
{
    return (unsigned)std::fmax(0, std::round(val));
}

template <typename t>
static cc_forceinline constexpr int signum(const t& x)
{
    return x < t{0} ? -1 : 1;
}
