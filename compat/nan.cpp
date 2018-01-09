#include "export.hpp"

#include "compat/macros.hpp"

#if defined(_MSC_VER)
#   include <float.h>
#   include <math.h>
#   define my_isnan ::_isnan
#   define my_isinf ::isinf
#elif defined __MINGW32__

static int __cdecl my_isnan(double)__asm__("__isnan");
static int __cdecl my_fpclassify(double)__asm__("___fpclassify");

#define FP_NAN          0x0100
#define FP_NORMAL       0x0400
#define FP_INFINITE     (FP_NAN | FP_NORMAL)
#define FP_ZERO         0x4000
#define FP_SUBNORMAL    (FP_NORMAL | FP_ZERO)

#define my_isinf(x) (my_fpclassify(x) == FP_INFINITE)

#elif defined __APPLE__
#   include <math.h>
#   define my_isnan(x) isnan(x)
#   define my_isinf(x) isinf(x)
#else
int my_isnan(double)__asm__("isnan");
int my_isinf(double)__asm__("isinf");

#endif
OTR_COMPAT_EXPORT never_inline bool nanp(double x)
{
    return my_isnan(x) || my_isinf(x);
}
