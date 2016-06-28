#pragma once

#if defined(__GNUC__)
bool __attribute__ ((noinline)) nanp(double value);
#elif defined(_WIN32)
__declspec(noinline) bool nanp(double value);
#else
bool nanp(double value);
#endif
