#pragma once

#ifdef BUILD_dinput
#   ifdef _WIN32
#       define OPENTRACK_DINPUT_LINKAGE __declspec(dllexport)
#   else
#       define OPENTRACK_DINPUT_LINKAGE
#   endif

#   ifndef _MSC_VER
#       define OPENTRACK_DINPUT_EXPORT __attribute__ ((visibility ("default"))) OPENTRACK_DINPUT_LINKAGE
#   else
#       define OPENTRACK_DINPUT_EXPORT OPENTRACK_DINPUT_LINKAGE
#   endif

#else
#ifdef _WIN32
#    define OPENTRACK_DINPUT_LINKAGE __declspec(dllimport)
#else
#    define OPENTRACK_DINPUT_LINKAGE
#endif

#ifndef _MSC_VER
#    define OPENTRACK_DINPUT_EXPORT __attribute__ ((visibility ("default"))) OPENTRACK_DINPUT_LINKAGE
#else
#    define OPENTRACK_DINPUT_EXPORT OPENTRACK_DINPUT_LINKAGE
#endif
#endif
