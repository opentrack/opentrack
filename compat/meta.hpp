#pragma once

/* Copyright (c) 2017 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include <type_traits>

template<typename t>
using cv_qualified = std::conditional_t<std::is_fundamental_v<std::decay_t<t>>,
                                        std::decay_t<t>,
                                        std::add_lvalue_reference_t<std::add_const_t<std::remove_reference_t<t>>>>;

#define progn(...) (([&]() { __VA_ARGS__ })())
#define prog1(x, ...) (([&]() { auto _ret1324 = (x); do { __VA_ARGS__; } while (0); return _ret1324; })())

#define once_only(...) do { static bool once__ = false; if (!once__) { once__ = true; __VA_ARGS__; } } while(false)

#if 0

#include <tuple>
#include <cstddef>

namespace meta {

namespace detail {

    template<typename x, typename y>
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

    template<template<typename...> class inst, typename x>
    struct lift_;

    template<template<typename...> class inst, template<typename...> class x, typename... xs>
    struct lift_<inst, x<xs...>>
    {
        using type = inst<xs...>;
    };
} // ns detail


template<typename... xs>
using reverse = typename detail::reverse_<std::tuple<xs...>, std::tuple<>>::type;

template<template<typename...> class inst, class x>
using lift = typename detail::lift_<inst, x>::type;

template<typename x, typename... xs>
using first = x;

template<typename x, typename... xs>
using rest = std::tuple<xs...>;

template<typename... xs>
using butlast = reverse<rest<reverse<xs...>>>;

template<typename... xs>
using last = lift<first, reverse<xs...>>;

template<std::size_t max>
using index_sequence = typename detail::index_sequence_<std::size_t, max, std::size_t(0)>::type;

} // ns meta

#endif
