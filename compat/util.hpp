#pragma once

#include "ndebug-guard.hpp"
#include "run-in-thread.hpp"
#include "meta.hpp"

#include <memory>
#include <cmath>
#include <utility>

#include <QSharedPointer>
#include <QDebug>

#define progn(...) ([&]() { __VA_ARGS__ }())

#define once_only(...) progn(static bool once = false; if (!once) { once = true; __VA_ARGS__; })

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

template<typename t>
int iround(const t& val)
{
    return int(std::round(val));
}

namespace util_detail {

template<typename n>
inline auto clamp_(n val, n min, n max) -> n
{
    if (val > max)
        return max;
    if (val < min)
        return min;
    return val;
}

}

template<typename t, typename u, typename w>
inline auto clamp(const t& val, const u& min, const w& max) -> decltype(val * min * max)
{
    return ::util_detail::clamp_<decltype(val * min * max)>(val, min, max);
}

template<typename t, typename... xs>
auto qptr(xs... args)
{
    return QSharedPointer<t>(new t(std::forward<xs>(args)...));
}

template<typename t> using qshared = QSharedPointer<t>;

#if defined _MSC_VER
#   define OTR_NEVER_INLINE __declspec(noinline)
#elif defined __GNUG__
#   define OTR_NEVER_INLINE __attribute__((noinline))
#else
#   define OTR_NEVER_INLINE
#endif

#if defined _MSC_VER || defined __GNUG__
#   define OTR_RESTRICT __restrict
#else
#   define OTR_RESTRICT
#endif

#if defined _MSC_VER
#   define OTR_ALWAYS_INLINE __forceinline
#elif defined __GNUG__
#   define OTR_ALWAYS_INLINE __attribute__((always_inline))
#else
#   define OTR_ALWAYS_INLINE inline
#endif

#if defined __GNUG__
#   define OTR_FLATTEN __attribute__((flatten))
#else
#   define OTR_FLATTEN OTR_ALWAYS_INLINE
#endif
