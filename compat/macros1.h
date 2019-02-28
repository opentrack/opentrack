#pragma once

#if defined _MSC_VER
#   define never_inline __declspec(noinline)
#else
#   define never_inline __attribute__((noinline))
#endif

#if defined _MSC_VER
#   define force_inline __forceinline
#else
#   define force_inline __attribute__((always_inline))
#endif

#if !defined likely
#   if defined __GNUC__
#      define likely(x)       __builtin_expect(!!(x),1)
#      define unlikely(x)     __builtin_expect(!!(x),0)
#   else
#      define likely(x) (x)
#      define unlikely(x) (x)
#   endif
#endif

#if defined _MSC_VER
#   define function_name __FUNCSIG__
#else
#   define function_name __PRETTY_FUNCTION__
#endif

#define PP_CAT(x,y) PP_CAT1(x,y)
#define PP_CAT1(x,y) x ## y

#define PP_EXPAND(x) PP_EXPAND2(x)
#define PP_EXPAND2(x) x

#ifdef _MSC_VER
#   define unreachable() do { __assume(0); *(volatile int*)nullptr = 0; } while (0) /* NOLINT(clang-analyzer-core.NullDereference) */
#else
#   define unreachable() do { __builtin_unreachable(); *(volatile int*)nullptr = 0; } while (0) /* NOLINT(clang-analyzer-core.NullDereference) */
#endif
