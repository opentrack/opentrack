#pragma once

#ifdef BUILD_compat
#   ifdef _WIN32
#       define OPENTRACK_LINKAGE __declspec(dllexport)
#   else
#       define OPENTRACK_LINKAGE
#   endif

#   ifndef _MSC_VER
#       define OPENTRACK_COMPAT_EXPORT __attribute__ ((visibility ("default"))) OPENTRACK_LINKAGE
#   else
#       define OPENTRACK_COMPAT_EXPORT OPENTRACK_LINKAGE
#   endif
#else
#   include "import.hpp"
#endif
