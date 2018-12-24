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
using to_const_ref_t = std::add_lvalue_reference_t<std::add_const_t<remove_cvref_t<t>>>;

// causes ICE in Visual Studio 2017 Preview. the ICE was reported and they handle them seriously in due time.
// the ICE is caused by decltype(auto) and const& return value
//#define eval_once(expr) ([&]() -> decltype(auto) { static decltype(auto) ret___1132 = (expr); return (decltype(ret___1132) const&) ret___1132; }())

#define eval_once(expr) eval_once2(expr, __COUNTER__)

#define eval_once2(expr, ctr)                                       \
    ([&] {                                                          \
        [[maybe_unused]]                                            \
        static auto PP_CAT(eval_once_, ctr) = (((void)(expr)), 0);  \
    }())

template<typename t>
using cv_qualified = std::conditional_t<std::is_fundamental_v<remove_cvref_t<t>>,
                                        remove_cvref_t<t>,
                                        to_const_ref_t<t>>;

#define progn(...) ([&]() -> decltype(auto) { __VA_ARGS__ }())

// end c++-only macros
#endif
