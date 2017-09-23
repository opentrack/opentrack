#pragma once

#include "util.hpp"

#include <type_traits>
#include <cinttypes>
#include <vector>
#include <array>
#include <iterator>

#include <QString>
#include <QVariant>

template<typename t, int M, typename size_type_ = std::uintptr_t>
struct powerset final
{
    static_assert(is_integral_v<size_type_>, "");

    using size_type = size_type_;

    static_assert(M > 0, "");
    static_assert(M < sizeof(size_type[8]), "");
    static_assert((is_unsigned_v<size_type>) || M < sizeof(size_type)*8 - 1, "");

    using N = std::integral_constant<size_type, (size_type(1) << size_type(M))-1>;
    static_assert((N::value & (N::value + 1)) == 0, "");

    using set_type = std::vector<t>;
    using sets_type = std::array<set_type, N::value>;
    using element_type = t;
    using element_count = std::integral_constant<size_type, N::value>;
    using self_type = powerset<t, M>;

    operator const sets_type&() const { return sets_; }
    operator sets_type&() { return sets_; }

    const sets_type& sets() const { return sets_; }
    sets_type& sets() { return sets_; }

    set_type& operator[](unsigned k) { return sets_[k]; }
    const set_type& operator[](unsigned k) const { return sets_[k]; }

    const set_type& elements() const { return elements_; }
    set_type& elements() { return elements_; }

    template<typename = void>
    operator QString() const
    {
        QString str;
        unsigned k = 0;
        for (const auto& set : sets_)
        {
            str.append(QStringLiteral("#%1: ").arg(++k));
            for (const auto& x : set)
                str.append(QStringLiteral("%1 ").arg(x));
            str.append('\n');
        }
        return str.mid(0, str.size() - 1);
    }

    powerset() {}

private:
    sets_type sets_;
    set_type elements_;
};

template<typename t, typename... xs>
auto make_powerset(const t& arg, const xs&... args)
{
    using cnt = std::integral_constant<std::uintptr_t, sizeof...(xs)+1>;
    using p = powerset<t, cnt::value>;
    using len = typename p::element_count;
    using vec = typename p::set_type;
    using size_type = typename p::size_type;

    p ret;
    vec v;
    v.reserve(len());

    const typename p::set_type ts {{arg, static_cast<t>(args)...}};

    ret.elements() = std::vector<t>(std::begin(ts), std::end(ts));

    // no nullary set
    for (size_type i = 0; i < len(); i++)
    {
        v.clear();
        size_type k = 1;
        for (const t& x : ts)
        {
            if ((i+1) & k)
                v.push_back(std::move(x));
            k <<= 1;
        }

        ret[i] = vec(std::begin(v), std::end(v));
        ret[i].shrink_to_fit();
    }

    return ret;
}
