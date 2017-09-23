#pragma once

#include "value-templates.hpp"

#include <algorithm>
#include <iterator>
#include <type_traits>
#include <vector>

namespace functools
{

template<typename seq_, typename = void>
struct reserver_
{
    static inline void maybe_reserve_space(seq_&, unsigned)
    {
        //qDebug() << "nada";
    }
};

template<typename seq_>
struct reserver_<seq_, decltype(std::declval<seq_>().reserve(0u), (void)0)>
{
    static inline void maybe_reserve_space(seq_& seq, unsigned sz)
    {
        seq.reserve(sz);
    }
};

template<typename seq_>
inline void maybe_reserve_space(seq_& seq, unsigned sz)
{
    reserver_<seq_, void>::maybe_reserve_space(seq, sz);
}

} // ns

template<typename t, t value_>
struct constant final
{
    using type = t;
    constexpr type operator()() const noexcept
    {
        return value_;
    }
    static constexpr type value = value_;

    constant() = delete;
};

template<typename seq_, typename F>
auto map(F&& fun, const seq_& seq)
{
    using value_type = std::decay_t<typename std::iterator_traits<decltype(std::begin(seq))>::value_type>;
    using ret_type = std::decay_t<decltype(fun(std::declval<value_type>()))>;

    std::vector<ret_type> ret;
    auto it = std::back_inserter(ret);

    for (const auto& elt : seq)
        it = fun(elt);

    return ret;
}

template<typename seq_, typename F>
auto remove_if_not(F&& fun, const seq_& seq)
{
    using namespace functools;

    using seq_type = std::decay_t<seq_>;
    using value_type = std::decay_t<typename std::iterator_traits<decltype(std::begin(seq))>::value_type>;
    using fun_ret_type = decltype(fun(std::declval<value_type>()));
    static_assert(is_convertible_v<fun_ret_type, bool>, "must return bool");

    seq_type ret;
    maybe_reserve_space(ret, seq.size());

    std::back_insert_iterator<seq_type> it = std::back_inserter(ret);

    for (const value_type& elt : seq)
        if (fun(elt))
            it = elt;

    return ret;
}
