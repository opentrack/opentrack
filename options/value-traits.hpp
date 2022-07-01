#pragma once

#include "slider.hpp"
#include "export.hpp"

#include <cmath>
#include <type_traits>

#include <QString>

namespace options::detail {
template<typename t>
using cv_qualified =
    std::conditional_t<std::is_fundamental_v<std::remove_cvref_t<t>>,
                       std::remove_cvref_t<t>,
                       std::add_lvalue_reference_t<std::add_const_t<std::remove_cvref_t<t>>>>;

template<typename t, typename = void>
struct maybe_enum_type {
    using type = t;
};
template<typename t>
struct maybe_enum_type<t, std::enable_if_t<std::is_enum_v<t>>> {
    using type = int;
};

template<typename t> using maybe_enum_type_t = typename maybe_enum_type<t>::type;

template<typename t, typename u = t, typename Enable = void>
struct default_value_traits
{
    using value_type = t;
    using stored_type = u;

    static inline
    value_type value_with_default(cv_qualified<value_type> val, cv_qualified<value_type>)
    {
        return val;
    }

    static inline
    value_type value_from_storage(cv_qualified<stored_type> x)
    {
        return static_cast<value_type>(x);
    }

    static inline
    stored_type storage_from_value(cv_qualified<value_type> val)
    {
        return static_cast<stored_type>(val);
    }

    static inline
    value_type value_from_qvariant(const QVariant& x)
    {
        return value_from_storage(storage_from_qvariant(x));
    }

    static inline
    QVariant qvariant_from_value(cv_qualified<value_type> val)
    {
        return qvariant_from_storage(storage_from_value(val));
    }

    static constexpr inline
    value_type pass_value(cv_qualified<value_type> x)
    {
        if constexpr(std::is_same_v<value_type, stored_type>)
            return x;
        else
            return value_from_storage(storage_from_value(x));
    }

    static inline
    stored_type storage_from_qvariant(const QVariant& x)
    {
        // XXX TODO
        return x.value<stored_type>();
    }

    static inline
    QVariant qvariant_from_storage(cv_qualified<stored_type> val)
    {
        // XXX TODO
        return QVariant::fromValue<stored_type>(val);
    }

    static inline
    bool is_equal(cv_qualified<value_type> x, cv_qualified<value_type> y)
    {
        return x == y;
    }
};

template<typename t, typename Enable = void>
struct value_traits : default_value_traits<t> {};

template<>
inline
bool default_value_traits<double>::is_equal(double x, double y)
{
    return std::fabs(x - y) < 1e-6;
}

template<> struct value_traits<float, double> : default_value_traits<float> {};

template<>
inline
bool default_value_traits<float, double>::is_equal(float x, float y)
{
    return std::fabs(x - y) < 1e-6f;
}

template<>
inline
slider_value default_value_traits<slider_value>::value_with_default(cv_qualified<slider_value> val, cv_qualified<slider_value> def)
{
    return { val.cur(), def.min(), def.max() };
}

template<>
inline
bool default_value_traits<slider_value>::is_equal(cv_qualified<slider_value> x, cv_qualified<slider_value> y)
{
    return value_traits<double>::is_equal(x.cur(), y.cur());
}

// Qt uses int a lot in slots so use it for all enums
template<typename t>
struct value_traits<t, std::enable_if_t<std::is_enum_v<t>>> : default_value_traits<t, int> {};

} // ns options::detail

