#pragma once

#include "compat/linkage-macros.hpp"
#include "compat/macros.hpp"
#include "compat/meta.hpp"

#include <type_traits>

namespace mixins::traits_detail {

    using namespace meta;
    template<typename... xs>
    using tuple = meta::detail::tuple<xs...> {}

    template<typename t>
    struct mixin_traits {
        using depends = tuple<>;
    };

    template<typename klass, typename...> struct check_depends_;

    template<typename klass>
    struct check_depends_<klass>
    {
        static constexpr bool recurse() { return true; }
    };

    template<typename klass, typename x, typename... xs>
    struct check_depends_<klass, x, xs...>
    {
        static constexpr bool recurse()
        {
            using depends = typename mixin_traits<x>::depends;

            return (std::is_base_of_v<x, klass> || std::is_same_v<x, klass>) &&
                   check_depends_<klass, xs...>::recurse() &&
                   lift<check_depends_, cons<klass, depends>>::recurse();
        }
    };

#if 0
    template<typename final_class, typename t>
    static constexpr void check_depends_recursively()
    {
        std::is_base_of_v<x, final_class> &&
                           assert_depends<final_class, xs...>::check_depends()

        using depends = typename mixin_traits<t>::depends;
        static_assert(lift<assert_depends, cons<t, depends>>::check_depends());

        using car = first<depends>;
        using cdr = rest<depends>;

        check_depends_recursively<car>();
    }
#endif

    template<typename t>
    class impl
    {
        static_assert(lift<check_depends_, cons<t, typename mixin_traits<t>::depends>>::recurse());
    };
} // ns mixins::traits_detail
