#pragma once

#include <algorithm>
#include <iterator>
#include <type_traits>

namespace functools
{

constexpr void return_void();

template<typename t>
using remove_qualifiers = std::remove_reference_t<std::remove_cv_t<t>>;

template<typename seq_, typename = void>
struct reserver_
{
    static inline void maybe_reserve_space(seq_&, unsigned)
    {
        //qDebug() << "nada";
    }
};

template<typename seq_>
struct reserver_<seq_, decltype(std::declval<seq_>().reserve(0u), return_void())>
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

template<typename seq_, typename F>
auto map(F&& fun, const seq_& seq)
{
    using seq_type = functools::remove_qualifiers<seq_>;

    seq_type ret;
    std::back_insert_iterator<seq_type> it = std::back_inserter(ret);

    for (const auto& elt : seq)
        it = fun(elt);

    return ret;
}

template<typename seq_, typename F>
auto remove_if_not(F&& fun, const seq_& seq)
{
    using namespace functools;

    using seq_type = remove_qualifiers<seq_>;
    using value_type = typename std::iterator_traits<decltype(std::begin(std::declval<seq_>()))>::value_type;
    using fun_ret_type = decltype(fun(std::declval<const value_type&>()));
    static_assert(std::is_convertible<fun_ret_type, bool>::value, "must return bool");

    seq_type ret;
    maybe_reserve_space(ret, seq.size());

    std::back_insert_iterator<seq_type> it = std::back_inserter(ret);

    for (const value_type& elt : seq)
        if (fun(elt))
            it = elt;

    return ret;
}
