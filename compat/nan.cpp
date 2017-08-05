#include "export.hpp"

#if defined(_MSC_VER)
#   include <cmath>
#   define my_isnan std::isnan
#   define my_isinf std::isinf
extern "C" OTR_COMPAT_EXPORT __declspec(noinline) bool nanp(double x)
#elif defined __MINGW32__

int __cdecl my_isnan(double)__asm__("__isnan");
int __cdecl my_fpclassify(double)__asm__("___fpclassify");

#define FP_NAN          0x0100
#define FP_NORMAL       0x0400
#define FP_INFINITE     (FP_NAN | FP_NORMAL)
#define FP_ZERO         0x4000
#define FP_SUBNORMAL    (FP_NORMAL | FP_ZERO)

#define my_isinf(x) (my_fpclassify(x) == FP_INFINITE)

extern "C" OTR_COMPAT_EXPORT bool __attribute__ ((noinline)) nanp(double x)
#elif defined __APPLE__
#   include <math.h>
#   define my_isnan(x) isnan(x)
#   define my_isinf(x) isinf(x)
extern "C" OTR_COMPAT_EXPORT bool __attribute__ ((noinline)) nanp(double x)
#else
int my_isnan(double)__asm__("isnan");
int my_isinf(double)__asm__("isinf");

extern "C" OTR_COMPAT_EXPORT bool __attribute__ ((noinline)) nanp(double x)
#endif
{
    return my_isnan(x) || my_isinf(x);
}
