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

using secs = duration<float>;
using ms = duration<float, std::milli>;
using us = duration<float, std::micro>;
using ns = duration<float, std::nano>;

} // ns time_units
