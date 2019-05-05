#pragma once

#include "log.hpp"

#if defined _MSC_VER && 0
// Get rid of annoying zero length structure warnings from libusb.h in MSVC
#   pragma warning(push)
#   pragma warning(disable : 4200)
#endif

#include "libusb.h"

#if defined _MSC_VER && 0
#   pragma warning(pop)
#endif

