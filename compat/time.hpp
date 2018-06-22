#pragma once

#include <chrono>

namespace time_units {

template<typename repr, typename ratio = std::ratio<1>>
using duration = std::chrono::duration<repr, ratio>;

template<typename t, typename u>
static inline constexpr auto time_cast(u&& in)
{
    return std::chrono::duration_cast<t>(in);
}

using secs = duration<double>;
using ms = duration<double, std::milli>;
using us = duration<double, std::micro>;
using ns = duration<double, std::nano>;

} // ns time_units
