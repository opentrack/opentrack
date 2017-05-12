#pragma once

#include <chrono>
#include <type_traits>

namespace time_units {

template<typename repr, typename ratio>
using duration = std::chrono::duration<repr, ratio>;

template<typename t, typename u>
static inline constexpr auto time_cast(const u& in)
{
    return std::chrono::duration_cast<t>(in);
}

using secs = duration<double, std::ratio<1, 1>>;
using secs_ = duration<long long, std::ratio<1, 1>>;
using ms = duration<double, std::milli>;
using ms_ = duration<long long, std::milli>;
using us = duration<double, std::micro>;
using us_ = duration<long long, std::micro>;
using ns = duration<long long, std::nano>;

} // ns time_units
