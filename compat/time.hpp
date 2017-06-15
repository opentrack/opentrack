#pragma once

#include "compat/functional.hpp"

#include <chrono>

namespace time_units {

template<typename repr, typename ratio = std::ratio<1>>
using duration = std::chrono::duration<repr, ratio>;

template<typename t, typename u>
static inline constexpr auto time_cast(const u& in)
{
    return std::chrono::duration_cast<t>(in);
}

using secs = duration<double>;
using secs_ = duration<long>;
using ms = duration<double, std::milli>;
using us = duration<double, std::micro>;
using us_ = duration<long long, std::micro>;
using ns = duration<long long, std::nano>;

} // ns time_units
