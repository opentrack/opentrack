#pragma once

#if defined _MSC_VER
#   define cc_noinline __declspec(noinline)
#else
#   define cc_noinline __attribute__((noinline))
#endif

#if defined _MSC_VER
#   define cc_forceinline __forceinline
#else
#   define cc_forceinline __attribute__((always_inline))
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
#   define cc_function_name __FUNCSIG__
#else
#   define cc_function_name __PRETTY_FUNCTION__
#endif

#if !defined PP_CAT
#   define PP_CAT(x,y) PP_CAT1(x,y)
#   define PP_CAT1(x,y) PP_CAT2(x,y)
#   define PP_CAT2(x,y) x ## y
#endif

#ifndef PP_EXPAND
#   define PP_EXPAND(x) PP_EXPAND2(x)
#   define PP_EXPAND2(x) PP_EXPAND3(x) x
#   define PP_EXPAND3(x) x
#endif

#ifdef _MSC_VER
//#   include <windows.h>
//#   define FULL_BARRIER MemoryBarrier()
#   define COMPILER_BARRIER() _ReadWriteBarrier()
#else
//#   define FULL_BARRIER() __sync_synchronize()
#   define COMPILER_BARRIER() asm volatile("" ::: "memory")
#endif
