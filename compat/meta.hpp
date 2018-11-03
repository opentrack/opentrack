#pragma once

/* Copyright (c) 2017 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

namespace meta::detail {

    template<typename... xs>
    struct tuple;

    template<typename... xs>
    struct reverse_;

    template<typename x0, typename... xs, template<typename...> class x, typename... ys>
    struct reverse_<x<x0, xs...>, x<ys...>>
    {
        using type = typename reverse_<x<xs...>, x<x0, ys...>>::type;
    };

    template<template<typename...> class x, typename... ys>
    struct reverse_<x<>, x<ys...>>
    {
        using type = x<ys...>;
    };

    template<template<typename...> class, typename, typename...> struct lift_;

    template<template<typename...> class to, template<typename...> class from, typename... xs>
    struct lift_<to, from<xs...>>
    {
        using type = to<xs...>;
    };

    template<typename> struct append_helper;

    template<typename, typename> struct cons_;

    template<typename x, template<typename...> class t, typename... xs>
    struct cons_<x, t<xs...>>
    {
        using type = t<x, xs...>;
    };

    template<typename> struct append2;

    template<template<typename...> class t, typename... xs>
    struct append2<t<xs...>>
    {
        template<typename> struct append1;

        template<template<typename...> class u, typename... ys>
        struct append1<u<ys...>>
        {
            using type = t<xs..., ys...>;
        };
    };

    template<typename, typename...> struct list__;

    template<typename rest, typename... first>
    struct list__
    {
        template<typename> struct list1;

        template<template<typename...> class t, typename... xs>
        struct list1<t<xs...>>
        {
            using type = t<first..., xs...>;
        };

        using type = typename list1<rest>::type;
    };

    template<typename xs, typename ys>
    struct append_
    {
        using t1 = append2<xs>;
        using type = typename t1::template append1<ys>::type;
    };

} // ns meta::detail

namespace meta {
    template<typename... xs>
    using tuple_ = detail::tuple<xs...>;

    template<typename... xs>
    using reverse = typename detail::reverse_<detail::tuple<xs...>, detail::tuple<>>::type;

    // the to/from order is awkward but mimics function composition
    template<template<typename...> class to, typename from>
    using lift = typename detail::lift_<to, from>::type;

    template<template<typename...> class to, typename from>
    constexpr inline auto lift_v = detail::lift_<to, from>::type::value;

    template<typename x, typename... xs>
    using first = x;

    template<typename x, typename... xs>
    using rest = detail::tuple<xs...>;

    template<typename... xs>
    using butlast = reverse<rest<reverse<xs...>>>;

    template<typename... xs>
    using last = lift<first, reverse<xs...>>;

    template<typename x, typename rest>
    using cons = typename detail::cons_<x, rest>::type;

    template<typename xs, typename ys>
    using append = typename detail::append_<xs, ys>;

    template<typename rest, typename... xs>
    using list_ = typename detail::list__<rest, xs...>;

} // ns meta

