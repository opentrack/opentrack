#pragma once

#if defined _WIN32
#   include "export.hpp"

#include <type_traits>

namespace timer_impl
{

// cf. https://msdn.microsoft.com/en-us/library/windows/desktop/aa383751%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
// for NTSTATUS, see http://stackoverflow.com/questions/3378622/how-to-understand-the-ntstatus-nt-success-typedef-in-windows-ddk

using ulong_ = unsigned long;
using long_ = long;
using byte_ = unsigned char;
using boolean_ = byte_;

using true_ = std::integral_constant<int, 1>;

using fail_ = std::integral_constant<byte_, byte_(-1)>;

// cf. http://stackoverflow.com/questions/3378622/how-to-understand-the-ntstatus-nt-success-typedef-in-windows-ddk

// typedef __success(return >= 0) LONG NTSTATUS;

// __success(expr) T f() :  indicates whether function f succeeded or
// not. If is true at exit, all the function's guarantees (as given
// by other annotations) must hold. If is false at exit, the caller
// should not expect any of the function's guarantees to hold. If not used,
// the function must always satisfy its guarantees. Added automatically to
// functions that indicate success in standard ways, such as by returning an
// HRESULT.

using ntstatus_ = long_;

// finally what we want
// this is equivalent to typedef syntax

using funptr_NtSetTimerResolution = ntstatus_ (__stdcall *)(ulong_, boolean_, ulong_*);

// RAII wrapper

class OTR_COMPAT_EXPORT timer_resolution final
{
    ulong_ old_value;

public:
    timer_resolution(int msecs);
    ~timer_resolution();
};

}

using timer_impl::timer_resolution;

#else
struct timer_resolution final
{
    inline timer_resolution(int) {}
};
#endif
