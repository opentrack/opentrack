#pragma once

#ifdef BUILD_api
#   ifdef _WIN32
#       define OPENTRACK_API_LINKAGE __declspec(dllexport)
#   else
#       define OPENTRACK_API_LINKAGE
#   endif

#   ifndef _MSC_VER
#       define OPENTRACK_API_EXPORT __attribute__ ((visibility ("default"))) OPENTRACK_API_LINKAGE
#   else
#       define OPENTRACK_API_EXPORT OPENTRACK_API_LINKAGE
#   endif

#else
    #ifdef _WIN32
    #    define OPENTRACK_API_LINKAGE __declspec(dllimport)
    #else
    #    define OPENTRACK_API_LINKAGE
    #endif

    #ifndef _MSC_VER
    #    define OPENTRACK_API_EXPORT __attribute__ ((visibility ("default"))) OPENTRACK_API_LINKAGE
    #else
    #    define OPENTRACK_API_EXPORT OPENTRACK_API_LINKAGE
    #endif
#endif
