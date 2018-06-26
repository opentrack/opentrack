#pragma once

#if defined _MSC_VER
#   define cc_noinline __declspec(noinline)
#elif defined __GNUG__
#   define cc_noinline __attribute__((noinline))
#else
#   define cc_noinline
#endif

#if defined _MSC_VER
#   define cc_forceinline __forceinline
#else
#   define cc_forceinline __attribute__((always_inline, gnu_inline)) inline
#endif

#ifdef Q_CREATOR_RUN
#   define cc_warn_unused_result
#elif defined _MSC_VER
#   define cc_warn_unused_result _Check_return_
#else
#   define cc_warn_unused_result __attribute__((warn_unused_result))
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
#   define cc_function_name (__FUNCSIG__)
#else
#   define cc_function_name (__PRETTY_FUNCTION__)
#endif

#if !defined PP_CAT
#   define PP_CAT(x,y) PP_CAT1(x,y)
#   define PP_CAT1(x,y) x##y
#endif

#define once_only(...) do { static bool once__ = false; if (!once__) { once__ = true; __VA_ARGS__; } } while(false)

#if defined __cplusplus

// from now only C++

#include <type_traits>

template<typename t>
using cv_qualified = std::conditional_t<std::is_fundamental_v<std::decay_t<t>>,
                                        std::decay_t<t>,
                                        std::add_lvalue_reference_t<std::add_const_t<std::remove_reference_t<t>>>>;

template<bool>
[[deprecated]] constexpr cc_forceinline void static_warn() {}

template<>
constexpr cc_forceinline void static_warn<true>() {}

#define static_warning(cond)            \
        static_warn<(cond)>();          \

#define progn(...) (([&] { __VA_ARGS__ })())
#define prog1(x, ...) (([&] { auto _ret1324 = (x); do { __VA_ARGS__; } while (0); return _ret1324; })())

// end c++-only macros
#endif

