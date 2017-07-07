#pragma once

#include <cmath>
#include <cinttypes>

namespace otr_math
{

using std::copysign;

using std::sqrt;
using std::pow;

using std::fabs;
using std::fmin;
using std::fmax;

using std::atan;
using std::atan2;
using std::asin;
using std::acos;

using std::sin;
using std::cos;
using std::tan;

using std::round;
using std::fmod;

using std::uintptr_t;
using std::intptr_t;

using std::int64_t;
using std::int32_t;

using std::uint64_t;
using std::uint32_t;

template <typename T>
static inline constexpr auto signum(T x)
{
    return (T() < x) - (x < T());
}

} // ns otr_math
