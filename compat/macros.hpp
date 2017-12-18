#pragma once

#define otr_tr(str) (QCoreApplication::translate(OTR_MODULE_NAME, (str)))

#if defined _MSC_VER
#   define never_inline __declspec(noinline)
#elif defined __GNUG__
#   define never_inline __attribute__((noinline))
#else
#   define never_inline
#endif

#if defined __GNUG__
#   define restrict_ptr __restrict
#elif defined _MSC_VER
#   define restrict_ptr __restrict
#else
#   define restrict_ptr
#endif

#if defined _MSC_VER
#   define restrict_ref restrict_ptr
#elif defined __GNUG__
#   define restrict_ref restrict_ptr
#else
#   define restrict_ref
#endif

#if defined _MSC_VER
#   define force_inline __forceinline
#elif defined __GNUG__
#   define force_inline __attribute__((always_inline, gnu_inline)) inline
#else
#   define force_inline inline
#endif

#ifdef Q_CREATOR_RUN
#   define warn_result_unused
#elif defined(_MSC_VER)
#   define warn_result_unused _Check_return_
#else
#   define warn_result_unused __attribute__((warn_unused_result))
#endif

#if defined(__GNUG__)
#   define unused(t, i) t __attribute__((unused)) i
#else
#   define unused(t, i) t
#endif

#if defined __GNUC__
#   define likely(x)       __builtin_expect(!!(x),1)
#   define unlikely(x)     __builtin_expect(!!(x),0)
#else
#   define likely(x) (x)
#   define unlikely(x) (x)
#endif
