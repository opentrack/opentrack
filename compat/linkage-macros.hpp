#pragma once

#if defined _MSC_VER || defined _WIN32
#   define OTR_GENERIC_EXPORT __declspec(dllexport)
#   define OTR_GENERIC_IMPORT __declspec(dllimport)
#else
#   define OTR_GENERIC_EXPORT __attribute__ ((visibility ("default")))
#   define OTR_GENERIC_IMPORT
#endif

#if defined _MSC_VER
#   define OTR_GENERIC_TEMPLATE
#else
#   define OTR_GENERIC_TEMPLATE __attribute__ ((visibility ("default")))
#endif
