#pragma once

#ifdef BUILD_cv
#   ifdef _WIN32
#       define OPENTRACK_CV_LINKAGE __declspec(dllexport)
#   else
#       define OPENTRACK_CV_LINKAGE
#   endif

#   ifndef _MSC_VER
#       define OPENTRACK_CV_EXPORT __attribute__ ((visibility ("default"))) OPENTRACK_CV_LINKAGE
#   else
#       define OPENTRACK_CV_EXPORT OPENTRACK_CV_LINKAGE
#   endif

#else
    #ifdef _WIN32
    #    define OPENTRACK_CV_LINKAGE __declspec(dllimport)
    #else
    #    define OPENTRACK_CV_LINKAGE
    #endif

    #ifndef _MSC_VER
    #    define OPENTRACK_CV_EXPORT __attribute__ ((visibility ("default"))) OPENTRACK_CV_LINKAGE
    #else
    #    define OPENTRACK_CV_EXPORT OPENTRACK_CV_LINKAGE
    #endif
#endif
