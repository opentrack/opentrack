#pragma once

#ifndef OTR_GENERIC_EXPORT
#   if defined _MSC_VER
#      define OTR_GENERIC_EXPORT __declspec(dllexport)
#      define OTR_GENERIC_IMPORT __declspec(dllimport)
#   elif defined _WIN32 && !defined __WINE__
#      define OTR_GENERIC_EXPORT __attribute__((dllexport, visibility ("default")))
#      define OTR_GENERIC_IMPORT __attribute__((dllimport))
#   else
#      define OTR_GENERIC_EXPORT __attribute__((visibility ("default")))
#      define OTR_GENERIC_IMPORT
#   endif
#endif

#if defined __APPLE__ || defined __MINGW32__
#   define OTR_NO_TMPL_INST // link failure on both targets
#endif

#if defined OTR_NO_TMPL_INST
#   define OTR_TEMPLATE_IMPORT(x)
#   define OTR_TEMPLATE_EXPORT(x)
#else
#   define OTR_TEMPLATE_IMPORT(x) extern template class OTR_GENERIC_IMPORT x;
#   define OTR_TEMPLATE_EXPORT(x) template class OTR_GENERIC_EXPORT x;
#endif
