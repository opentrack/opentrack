#pragma once

#if !defined __WINE__
#   include <QCoreApplication>
#   define otr_tr(...) (QCoreApplication::translate(OTR_MODULE_NAME, __VA_ARGS__))
#   define _(...) (otr_tr(__VA_ARGS__))
#endif

#if defined _MSC_VER
#
#   define MEMORY_BARRIER _ReadWriteBarrier()
#else
#   define MEMORY_BARRIER asm volatile("" ::: "memory")
#endif

#if defined _MSC_VER
#   define never_inline __declspec(noinline)
#elif defined __GNUG__
#   define never_inline __attribute__((noinline))
#else
#   define never_inline
#endif

#if defined __cplusplus
#   define restrict_ptr __restrict
#endif

#if defined _MSC_VER
#   define force_inline __forceinline
#else
#   define force_inline __attribute__((always_inline, gnu_inline)) inline
#endif

#ifdef Q_CREATOR_RUN
#   define warn_result_unused
#elif defined _MSC_VER
#   define warn_result_unused _Check_return_
#else
#   define warn_result_unused __attribute__((warn_unused_result))
#endif

#if defined __GNUC__
#   define likely(x)       __builtin_expect(!!(x),1)
#   define unlikely(x)     __builtin_expect(!!(x),0)
#else
#   define likely(x) (x)
#   define unlikely(x) (x)
#endif

#if defined _MSC_VER
#   define OTR_FUNNAME (__FUNCSIG__)
#else
#   define OTR_FUNNAME (__PRETTY_FUNCTION__)
#endif
