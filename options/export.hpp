#pragma once

#ifdef BUILD_options
#   ifdef _WIN32
#       define OPENTRACK_OPTIONS_LINKAGE __declspec(dllexport)
#   else
#       define OPENTRACK_OPTIONS_LINKAGE
#   endif

#   ifndef _MSC_VER
#       define OPENTRACK_OPTIONS_EXPORT __attribute__ ((visibility ("default"))) OPENTRACK_OPTIONS_LINKAGE
#   else
#       define OPENTRACK_OPTIONS_EXPORT OPENTRACK_OPTIONS_LINKAGE
#   endif
#else
    #ifdef _WIN32
    #    define OPENTRACK_OPTIONS_LINKAGE __declspec(dllimport)
    #else
    #    define OPENTRACK_OPTIONS_LINKAGE
    #endif

    #ifndef _MSC_VER
    #    define OPENTRACK_OPTIONS_EXPORT __attribute__ ((visibility ("default"))) OPENTRACK_OPTIONS_LINKAGE
    #else
    #    define OPENTRACK_OPTIONS_EXPORT OPENTRACK_OPTIONS_LINKAGE
    #endif
#endif
