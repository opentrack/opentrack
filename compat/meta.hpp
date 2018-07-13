#pragma once

/* Copyright (c) 2017 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#define OTR_META_INST_FAIL(x)                                        \
    static_assert(sizeof...(x) == ~0ul);                             \
    static_assert(sizeof...(x) == 0u)

namespace meta::detail {

    template<typename... xs>
    struct tuple;

    template<typename... xs>
    struct reverse_
    {
        OTR_META_INST_FAIL(xs);
    };

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

    template<template<typename...> class inst, typename... xs>
    struct lift_
    {
        OTR_META_INST_FAIL(xs);
    };

    template<template<typename...> class to, template<typename...> class from, typename... xs>
    struct lift_<to, from<xs...>>
    {
        using type = to<xs...>;
    };

    template<typename...> struct cons_;

    template<template<typename...> class t, typename x, typename... xs>
    struct cons_<t<xs...>, x>
    {
        using type = t<x, xs...>;
    };

} // ns meta::detail

namespace meta {
    template<typename... xs>
    using reverse = typename detail::reverse_<detail::tuple<xs...>, detail::tuple<>>::type;

    // the to/from order is awkward but mimics function composition
    template<template<typename...> class to, typename from>
    using lift = typename detail::lift_<to, from>::type;

    template<typename x, typename... xs>
    using first = x;

    template<typename x, typename... xs>
    using rest = detail::tuple<xs...>;

    template<typename... xs>
    using butlast = reverse<rest<reverse<xs...>>>;

    template<typename... xs>
    using last = lift<first, reverse<xs...>>;

    template<typename... xs>
    using cons = detail::cons_<xs...>;

} // ns meta

