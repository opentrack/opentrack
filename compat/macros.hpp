#pragma once

#include "macros1.h"

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

template<typename t>
using cv_qualified = std::conditional_t<sizeof(remove_cvref_t<t>) <= sizeof(void*)*4 && std::is_trivially_copyable_v<t> ||
                                        std::is_fundamental_v<remove_cvref_t<t>>,
                                        remove_cvref_t<t>,
                                        to_const_ref_t<t>>;
