#pragma once

#include <cmath>
#include <type_traits>

#include "macros.hpp"

namespace util_detail {

template<typename n>
inline auto clamp_float(n val, n min_, n max_)
{
    return std::fmin(std::fmax(val, min_), max_);
}

template<typename t, typename n>
struct clamp final
{
    static inline auto clamp_(const n& val, const n& min_, const n& max_)
    {
        if (unlikely(val > max_))
            return max_;
        if (unlikely(val < min_))
            return min_;
        return val;
    }
};

template<typename t>
struct clamp<float, t>
{
    static inline auto clamp_(float val, float min_, float max_)
    {
        return clamp_float(val, min_, max_);
    }
};

template<typename t>
struct clamp<double, t>
{
    static inline auto clamp_(double val, double min_, double max_)
    {
        return clamp_float(val, min_, max_);
    }
};

} // ns util_detail

template<typename t, typename u, typename w>
inline auto clamp(const t& val, const u& min, const w& max)
{
    using tp = decltype(val + min + max);
    return ::util_detail::clamp<std::decay_t<tp>, tp>::clamp_(val, min, max);
}
