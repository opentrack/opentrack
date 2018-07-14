#pragma once

#include "compat/linkage-macros.hpp"
#include "compat/macros.hpp"
#include "compat/meta.hpp"

#include <type_traits>

//namespace mixins::traits_detail {

    using namespace meta;
    template<typename... xs>
    using tuple = meta::detail::tuple<xs...>;

    template<typename t>
    struct mixin_traits {
        //using depends = tuple<>;
    };

    template<typename klass, typename...> struct check_depends_;

    template<typename klass>
    struct check_depends_<klass>
    {
        using type = std::bool_constant<true>;
    };

    template<typename klass, typename x, typename... xs>
    struct check_depends_<klass, x, xs...>
    {
        using b1 = std::is_base_of<x, klass>;
        using b2 = typename check_depends_<klass, xs...>::type;

        using depends = typename mixin_traits<x>::depends;
        using t1 = cons<klass, depends>;
        using t2 = lift<check_depends_, t1>;
        using b3 = typename t2::type;

        using type = std::bool_constant<b1::value && b2::value && b3::value>;
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
        using t1 = typename lift<check_depends_, cons<t, typename mixin_traits<t>::depends>>::type;
        static_assert(t1::value);
    };
//} // ns mixins::traits_detail
