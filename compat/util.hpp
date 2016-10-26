#pragma once

#include "make-unique.hpp"
#include "run-in-thread.hpp"

#include <memory>
#include <cmath>

#define progn(...) ([&]() { __VA_ARGS__ }())
template<typename t> using mem = std::shared_ptr<t>;
template<typename t> using ptr = std::unique_ptr<t>;

#if defined(_MSC_VER) && !defined(Q_CREATOR_RUN)
#   define DEFUN_WARN_UNUSED _Check_return_
#else
#   define DEFUN_WARN_UNUSED __attribute__((warn_unused_result))
#endif

template<typename t>
int iround(const t& val)
{
    return int(std::round(val));
}

namespace detail {

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
    return ::detail::clamp_<decltype(val * min * max)>(val, min, max);
}
