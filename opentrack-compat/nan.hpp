#pragma once

#include "export.hpp"

#if defined(__GNUC__)
extern "C" OPENTRACK_COMPAT_EXPORT bool __attribute__ ((noinline)) nanp(double value);
#elif defined(_WIN32)
extern "C" __declspec(noinline) OPENTRACK_COMPAT_EXPORT bool nanp(double value);
#else
extern "C" OPENTRACK_COMPAT_EXPORT bool nanp(double value);
#endif
