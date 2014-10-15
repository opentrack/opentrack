#pragma once
#ifdef _WIN32
#   define OPENTRACK_LINKAGE __declspec(dllexport)
#else
#   define OPENTRACK_LINKAGE
#endif
#define OPENTRACK_EXPORT __attribute__ ((visibility ("default"))) OPENTRACK_LINKAGE
