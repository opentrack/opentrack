#pragma once

#ifdef _WIN32
#    define OPENTRACK_COMPAT_LINKAGE __declspec(dllimport)
#else
#    define OPENTRACK_COMPAT_LINKAGE
#endif

#ifndef _MSC_VER
#    define OPENTRACK_COMPAT_EXPORT __attribute__ ((visibility ("default"))) OPENTRACK_COMPAT_LINKAGE
#else
#    define OPENTRACK_COMPAT_EXPORT OPENTRACK_COMPAT_LINKAGE
#endif
