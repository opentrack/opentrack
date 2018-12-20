#pragma once

#include "macros1.h"

#ifdef __cplusplus

// from now only C++

#include <utility>
#include <type_traits>

// before C++20
namespace cxx20_compat {
    template<typename T>
    struct remove_cvref {
        using type = std::remove_cv_t<std::remove_reference_t<T>>;
    };
} // ns cxx20_compat

template<typename t>
using remove_cvref_t = typename cxx20_compat::remove_cvref<t>::type;

template<typename t>
using to_const_cvref_t = std::add_lvalue_reference_t<std::add_const_t<remove_cvref_t<t>>>;

// causes ICE in Visual Studio 2017 Preview. the ICE was reported and they handle them seriously in due time.
// the ICE is caused by decltype(auto) and const& return value
//#define eval_once(expr) ([&]() -> decltype(auto) { static decltype(auto) ret___1132 = (expr); return (decltype(ret___1132) const&) ret___1132; }())

#define eval_once(expr) eval_once__2(expr, PP_CAT(_EVAL_ONCE__, __COUNTER__))
#define eval_once__2(expr, ident) eval_once__3(expr, ident)

#define eval_once__3(expr, ident)                                                               \
    ([&]() -> decltype(auto) {                                                                  \
        static auto INIT##ident = (expr);                                                       \
        return static_cast<to_const_cvref_t<decltype(INIT##ident)>>(INIT##ident);    \
    }())

#include <type_traits>

template<typename t>
using cv_qualified = std::conditional_t<std::is_fundamental_v<remove_cvref_t<t>>,
                                        remove_cvref_t<t>,
                                        to_const_cvref_t<t>>;

template<bool>
[[deprecated]] constexpr cc_forceinline void static_warn() {}

template<>
constexpr cc_forceinline void static_warn<true>() {}

#define static_warning(cond)            \
        static_warn<(cond)>();          \

#define progn(...) ([&]() -> decltype(auto) { __VA_ARGS__ }())

// end c++-only macros
#endif
