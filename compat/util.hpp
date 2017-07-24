#pragma once

#include "ndebug-guard.hpp"
#include "run-in-thread.hpp"
#include "meta.hpp"
#include "functional.hpp"

#include <memory>
#include <cmath>
#include <utility>

#include <QSharedPointer>
#include <QDebug>

#define progn(...) (([&]() { __VA_ARGS__ })())
#define prog1(x, ...) (([&]() { auto _ret1324 = (x); do { __VA_ARGS__; } while (0); return _ret1324; })())

#define once_only(...) progn(static bool once = false; if (!once) { once = true; __VA_ARGS__; })
#define load_time_value(x) \
    progn( \
        static const auto _value132((x)); \
        return static_cast<decltype(_value132)&>(value132); \
    )

template<typename t> using mem = std::shared_ptr<t>;
template<typename t> using ptr = std::unique_ptr<t>;

#ifdef Q_CREATOR_RUN
#   define DEFUN_WARN_UNUSED
#elif defined(_MSC_VER)
#   define DEFUN_WARN_UNUSED _Check_return_
#else
#   define DEFUN_WARN_UNUSED __attribute__((warn_unused_result))
#endif

#if defined(__GNUG__)
#   define unused(t, i) t __attribute__((unused)) i
#else
#   define unused(t, i) t
#endif

#if !defined(_WIN32)
#   define unused_on_unix(t, i) unused(t, i)
#else
#   define unused_on_unix(t, i) t i
#endif

#if defined __GNUC__
#   define likely(x)       __builtin_expect(!!(x),1)
#   define unlikely(x)     __builtin_expect(!!(x),0)
#else
#   define likely(x) (x)
#   define unlikely(x) (x)
#endif

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
inline auto clamp_(n val, n min, n max) -> n
{
    if (unlikely(val > max))
        return max;
    if (unlikely(val < min))
        return min;
    return val;
}

}

template<typename t, typename u, typename w>
inline auto clamp(const t& val, const u& min, const w& max) -> decltype(val + min + max)
{
    return ::util_detail::clamp_<decltype(val + min + max)>(val, min, max);
}

template<typename t, typename... xs>
auto qptr(xs... args)
{
    return QSharedPointer<t>(new t(std::forward<xs>(args)...));
}

template<typename t> using qshared = QSharedPointer<t>;

#if defined _MSC_VER
#   define never_inline __declspec(noinline)
#elif defined __GNUG__
#   define never_inline __attribute__((noinline))
#else
#   define never_inline
#endif

#if defined _MSC_VER || defined __GNUG__
#   define restrict __restrict
#else
#   define restrict
#endif

#if defined _MSC_VER
#   define restrict_ref restrict
#elif defined __GNUG__
#   define restrict_ref restrict
#else
#   define restrict_ref
#endif

#if defined _MSC_VER
#   define force_inline __forceinline
#elif defined __GNUG__
#   define force_inline __attribute__((always_inline, gnu_inline))
#else
#   define force_inline inline
#endif

#if defined __GNUG__
#   define flatten __attribute__((flatten, noinline))
#else
#   define flatten
#endif
