#pragma once

#include "export.hpp"

#if defined(__GNUC__)
OTR_COMPAT_EXPORT bool __attribute__ ((noinline)) nanp(double value);
#elif defined(_WIN32)
OTR_COMPAT_EXPORT __declspec(noinline) bool nanp(double value);
#else
OTR_COMPAT_EXPORT bool nanp(double value);
#endif
