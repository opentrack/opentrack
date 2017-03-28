#pragma once

#ifdef BUILD_spline_widget
#   ifdef _WIN32
#       define OPENTRACK_SPLINE_LINKAGE __declspec(dllexport)
#   else
#       define OPENTRACK_SPLINE_LINKAGE
#   endif

#   ifndef _MSC_VER
#       define OPENTRACK_SPLINE_EXPORT __attribute__ ((visibility ("default"))) OPENTRACK_SPLINE_LINKAGE
#   else
#       define OPENTRACK_SPLINE_EXPORT OPENTRACK_SPLINE_LINKAGE
#   endif
#else
    #ifdef _WIN32
    #    define OPENTRACK_SPLINE_LINKAGE __declspec(dllimport)
    #else
    #    define OPENTRACK_SPLINE_LINKAGE
    #endif

    #ifndef _MSC_VER
    #    define OPENTRACK_SPLINE_EXPORT __attribute__ ((visibility ("default"))) OPENTRACK_SPLINE_LINKAGE
    #else
    #    define OPENTRACK_SPLINE_EXPORT OPENTRACK_SPLINE_LINKAGE
    #endif
#endif
