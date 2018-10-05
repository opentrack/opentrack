#pragma once

#if defined _MSC_VER
#   define OTR_GENERIC_EXPORT __declspec(dllexport)
#   define OTR_GENERIC_IMPORT __declspec(dllimport)
#elif defined _WIN32 && !defined __WINE__
#   define OTR_GENERIC_EXPORT __attribute__((dllexport, visibility ("default")))
#   define OTR_GENERIC_IMPORT __attribute__((dllimport, visibility ("default")))
#else
#   define OTR_GENERIC_EXPORT __attribute__ ((visibility ("default")))
#   define OTR_GENERIC_IMPORT
#endif

#if defined __APPLE__
#   define OTR_TEMPLATE_IMPORT(x) //nothing
#else
#   define OTR_TEMPLATE_IMPORT(x) extern template class OTR_GENERIC_IMPORT x
#endif

#define OTR_TEMPLATE_EXPORT_(x) template class OTR_GENERIC_EXPORT x

#if defined __APPLE__
#   define OTR_TEMPLATE_EXPORT(x) // nothing
#else /* does this _always_ work for binutils ELF? */
#   define OTR_TEMPLATE_EXPORT(x) OTR_TEMPLATE_EXPORT_(x)
#endif
