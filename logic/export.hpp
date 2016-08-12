#pragma once

#ifdef BUILD_logic
#   ifdef _WIN32
#       define OPENTRACK_LOGIC_LINKAGE __declspec(dllexport)
#   else
#       define OPENTRACK_LOGIC_LINKAGE
#   endif

#   ifndef _MSC_VER
#       define OPENTRACK_LOGIC_EXPORT __attribute__ ((visibility ("default"))) OPENTRACK_LOGIC_LINKAGE
#   else
#       define OPENTRACK_LOGIC_EXPORT OPENTRACK_LOGIC_LINKAGE
#   endif

#else
#ifdef _WIN32
#    define OPENTRACK_LOGIC_LINKAGE __declspec(dllimport)
#else
#    define OPENTRACK_LOGIC_LINKAGE
#endif

#ifndef _MSC_VER
#    define OPENTRACK_LOGIC_EXPORT __attribute__ ((visibility ("default"))) OPENTRACK_LOGIC_LINKAGE
#else
#    define OPENTRACK_LOGIC_EXPORT OPENTRACK_LOGIC_LINKAGE
#endif
#endif
