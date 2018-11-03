#pragma once

#include "compat/meta.hpp"

#include <type_traits>

namespace mixins::traits_detail {

    using namespace meta;

    template<typename... xs> using tuple = tuple_<xs...>;

    template<typename t>
    struct mixin_traits {
        // implement this!
        //using depends = tuple<>;
    };

    template<typename klass, typename...> struct check_depends_;

    template<typename klass>
    struct check_depends_<klass> : std::true_type
    {
    };

    template<typename klass, typename x, typename... xs>
    struct check_depends_<klass, x, xs...> :
            std::bool_constant<
                std::is_base_of_v<x, klass> &&
                lift_v<check_depends_, cons<klass, typename mixin_traits<x>::depends>> &&
                check_depends_<klass, xs...>::value
            >
    {
    };

    template<typename klass, typename... xs>
    struct impl
    {
        static_assert(lift<check_depends_, tuple<klass, xs...>>::value,
                      "class must inherit dependent mixins");
    };
} // ns mixins::traits_detail
