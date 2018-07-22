#pragma once

#include "compat/meta.hpp"

#include <type_traits>

namespace mixins::traits_detail {

    using namespace meta;

    template<typename... xs> using tuple = tuple_<xs...>;

    template<typename t>
    struct mixin_traits {
        // implement this!
        using depends = tuple<>;

        // unconditional but at instantiation time
        static_assert(sizeof(t) < sizeof(char),
                      "must specialize mixin_traits");
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
                lift<check_depends_, cons<klass, typename mixin_traits<x>::depends>>::value &&
                check_depends_<klass, xs...>::value
            >
    {
    };

    template<typename klass, typename... xs>
    struct impl
    {
        static constexpr bool class_must_inherit_dependent_mixins =
                lift<check_depends_, tuple<klass, xs...>>::value;
        static_assert(class_must_inherit_dependent_mixins);
    };
} // ns mixins::traits_detail
