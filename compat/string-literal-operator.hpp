#pragma once

#include <QLatin1String>
#include <cstddef>

static inline constexpr auto operator"" _qstr(const char* str, std::size_t N)
{
    return QLatin1String(str, str + N);
}
