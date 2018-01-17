#pragma once

#include "opentrack-library-path.h"
#include "ndebug-guard.hpp"
#include "run-in-thread.hpp"
#include "meta.hpp"
#include "functional.hpp"
#include "macros.hpp"
#include "value-templates.hpp"

#include <type_traits>
#include <memory>
#include <cmath>
#include <utility>

#include <iterator>

#include <QDebug>

#define progn(...) (([&]() { __VA_ARGS__ })())
#define prog1(x, ...) (([&]() { auto _ret1324 = (x); do { __VA_ARGS__; } while (0); return _ret1324; })())

#define once_only(...) do { static bool once = false; if (!once) { once = true; __VA_ARGS__; } } while(false)

template<typename t>
inline int iround(const t& val)
{
    return int(std::round(val));
}

template<typename t>
inline unsigned uround(const t& val)
{
    return std::round(std::fmax(t(0), val));
}

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

template<typename t>
using cv_qualified = std::conditional_t<is_fundamental_v<std::decay_t<t>>, std::decay_t<t>, const t&>;


