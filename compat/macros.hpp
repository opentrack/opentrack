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

#if defined _MSC_VER
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
#   define PP_CAT1(x,y) PP_CAT2(x,y)
#   define PP_CAT2(x,y) x ## y
#endif

#if defined __cplusplus

// from now only C++

#include <utility>

//#define once_only(...) do { static bool once__ = false; if (!once__) { once__ = true; __VA_ARGS__; } } while(false)
//#define once_only(expr) ([&] { static decltype(auto) ret___1132 = (expr); return (decltype(ret___1132) const&) ret___1132; }())

#define eval_once__2(expr, ident) (([&] { static bool ident = ((expr), true); (void)(ident); }))
#define eval_once(expr) eval_once__2(expr, PP_CAT(eval_once_init__, __COUNTER__))

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
#define prog1(x, ...) (([&] { decltype(auto) ret1324 = (x); __VA_ARGS__; return ret1324; })())

// end c++-only macros
#endif

