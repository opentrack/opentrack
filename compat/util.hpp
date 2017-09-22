#pragma once

#include "ndebug-guard.hpp"
#include "run-in-thread.hpp"
#include "meta.hpp"
#include "functional.hpp"
#include "macros.hpp"

#include <memory>
#include <cmath>
#include <utility>

#include <QSharedPointer>
#include <QDebug>

#define progn(...) (([&]() { __VA_ARGS__ })())
#define prog1(x, ...) (([&]() { auto _ret1324 = (x); do { __VA_ARGS__; } while (0); return _ret1324; })())

#define once_only(...) do { static bool once = false; if (!once) { once = true; __VA_ARGS__; } } while(false)
#define load_time_value(x) \
    progn( \
        static const auto _value132((x)); \
        return static_cast<decltype(_value132)&>(value132); \
    )

template<typename t> using mem = std::shared_ptr<t>;
template<typename t> using ptr = std::unique_ptr<t>;

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
inline auto clamp_float(n val, n min, n max)
{
    return std::fmin(std::fmax(val, min), max);
}

template<typename t, typename n>
struct clamp final
{
    static inline auto clamp_(const n& val, const n& min, const n& max)
    {
        if (unlikely(val > max))
            return max;
        if (unlikely(val < min))
            return min;
        return val;
    }
};

template<typename t>
struct clamp<float, t>
{
    static inline auto clamp_(float val, float min, float max)
    {
        return clamp_float(val, min, max);
    }
};

template<typename t>
struct clamp<double, t>
{
    static inline auto clamp_(double val, double min, double max)
    {
        return clamp_float(val, min, max);
    }
};

} // ns util_detail

template<typename t, typename u, typename w>
inline auto clamp(const t& val, const u& min, const w& max)
{
    using tp = decltype(val + min + max);
    return ::util_detail::clamp<std::decay_t<tp>, tp>::clamp_(val, min, max);
}
