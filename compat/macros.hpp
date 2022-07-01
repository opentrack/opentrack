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
