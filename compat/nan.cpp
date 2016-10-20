#include "export.hpp"

#if defined(_MSC_VER)
#   include <cmath>
#   define my_isnan std::isnan
#   define my_isinf std::isinf
extern "C" OPENTRACK_COMPAT_EXPORT __declspec(noinline) bool nanp(double x)
#else
int my_isnan(double)__asm__("isnan");
int my_isinf(double)__asm__("isinf");

extern "C" OPENTRACK_COMPAT_EXPORT bool __attribute__ ((noinline)) nanp(double x)
#endif
{
    return my_isnan(x) || my_isinf(x);
}
