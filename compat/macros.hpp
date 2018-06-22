#pragma once

#if defined _MSC_VER
#   define MEMORY_BARRIER() _ReadWriteBarrier()
#else
#   define MEMORY_BARRIER() asm volatile("" ::: "memory")
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

#if !defined PP_CAT
#   define PP_CAT(x,y) PP_CAT1(x,y)
#   define PP_CAT1(x,y) x##y
#endif

#if defined __cplusplus

// from now only C++ macros

template<bool>
[[deprecated]] constexpr force_inline void static_warn() {}

template<>
constexpr force_inline void static_warn<true>() {}

#define static_warning(cond)            \
        static_warn<(cond)>();          \

#define progn(...) (([&] { __VA_ARGS__ })())
#define prog1(x, ...) (([&] { auto _ret1324 = (x); do { __VA_ARGS__; } while (0); return _ret1324; })())

// end c++-only macros
#endif

#define once_only(...) do { static bool once__ = false; if (!once__) { once__ = true; __VA_ARGS__; } } while(false)
