#pragma once

#include <type_traits>
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

    template<typename N, N max, N pos, typename... xs>
    struct index_sequence_
    {
        using part = std::integral_constant<N, pos>;
        using type = typename index_sequence_<N, max, pos+1, xs..., part>::type;
    };

    template<typename N, N max, typename... xs>
    struct index_sequence_<N, max, max, xs...>
    {
        using type = std::tuple<xs...>;
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
