#pragma once

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

#if defined __GNUG__
#   define flatten __attribute__((flatten, noinline))
#else
#   define flatten
#endif

#ifdef Q_CREATOR_RUN
#   define DEFUN_WARN_UNUSED
#elif defined(_MSC_VER)
#   define DEFUN_WARN_UNUSED _Check_return_
#else
#   define DEFUN_WARN_UNUSED __attribute__((warn_unused_result))
#endif

#if defined(__GNUG__)
#   define unused(t, i) t __attribute__((unused)) i
#else
#   define unused(t, i) t
#endif

#if !defined(_WIN32)
#   define unused_on_unix(t, i) unused(t, i)
#else
#   define unused_on_unix(t, i) t i
#endif

#if defined __GNUC__
#   define likely(x)       __builtin_expect(!!(x),1)
#   define unlikely(x)     __builtin_expect(!!(x),0)
#else
#   define likely(x) (x)
#   define unlikely(x) (x)
#endif
