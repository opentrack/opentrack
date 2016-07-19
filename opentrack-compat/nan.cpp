#include <cmath>

#if defined(__GNUC__)
extern "C" bool __attribute__ ((noinline)) nanp(double value)
#elif defined(_WIN32)
extern "C" __declspec(noinline) bool nanp(double value)
#else
extern "C" bool nanp(double value)
#endif
{
    using std::isnan;
    using std::isinf;

    const volatile double x = value;
    return isnan(x) || isinf(x);
}
