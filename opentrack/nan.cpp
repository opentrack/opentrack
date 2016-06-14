#include <cmath>

#if defined(__GNUC__)
bool __attribute__ ((noinline)) nanp(double value)
#elif defined(_WIN32)
__declspec(noinline) bool nanp(double value)
#else
bool nanp(double value)
#endif
{
    using std::isnan;
    using std::isinf;

    const volatile double x = value;
    return isnan(x) || isinf(x);
}