#pragma once

#if defined _MSC_VER
#   define OTR_GENERIC_EXPORT __declspec(dllexport)
#   define OTR_GENERIC_IMPORT __declspec(dllimport)
#elif defined _WIN32 && !defined(__WINE__)
#   define OTR_GENERIC_EXPORT __attribute__((dllexport, visibility ("default")))
#   define OTR_GENERIC_IMPORT __attribute__((dllimport))
#else
#   define OTR_GENERIC_EXPORT __attribute__ ((visibility ("default")))
#   define OTR_GENERIC_IMPORT
#endif

#define OTR_TEMPLATE_EXPORT template class OTR_GENERIC_EXPORT
#define OTR_TEMPLATE_IMPORT extern template class OTR_GENERIC_IMPORT
