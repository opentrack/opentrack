#pragma once

#ifndef OPENTRACK_EXPORT
#   ifdef _WIN32
#       define OPENTRACK_LINKAGE __declspec(dllimport)
#   else
#       define OPENTRACK_LINKAGE
#   endif

#   ifndef _MSC_VER
#       define OPENTRACK_EXPORT __attribute__ ((visibility ("default"))) OPENTRACK_LINKAGE
#   else
#       define OPENTRACK_EXPORT OPENTRACK_LINKAGE
#   endif
#endif
