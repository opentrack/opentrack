#pragma once

#include "slider.hpp"
#include "export.hpp"

#include <QString>

#include <cmath>
#include <cinttypes>
#include <type_traits>

namespace options::detail {

template<typename t, typename Enable = void>
struct value_traits;

template<typename t, typename u = t, typename Enable = void>
struct default_value_traits
{
    using stored_type = std::decay_t<u>;
    using value_type = std::decay_t<t>;

    using self = value_traits<t>;

    static value_type value_with_default(const value_type& val, const value_type&)
    {
        return val;
    }

    static value_type value_from_storage(const stored_type& x)
    {
        return static_cast<value_type>(x);
    }

    static stored_type storage_from_value(const value_type& val)
    {
        return static_cast<stored_type>(val);
    }

    static value_type value_from_qvariant(const QVariant& x)
    {
        return self::value_from_storage(self::storage_from_qvariant(x));
    }

    static QVariant qvariant_from_value(const value_type& val)
    {
        return self::qvariant_from_storage(self::storage_from_value(val));
    }

    static constexpr inline
    value_type pass_value(const value_type& x)
    {
        if constexpr(std::is_same_v<value_type, stored_type>)
            return x;
        else
            return self::value_from_storage(self::storage_from_value(x));
    }

    static stored_type storage_from_qvariant(const QVariant& x)
    {
        // XXX TODO
        return x.value<stored_type>();
    }

    static QVariant qvariant_from_storage(const stored_type& val)
    {
        // XXX TODO
        return QVariant::fromValue<stored_type>(val);
    }

    static bool is_equal(const value_type& x, const value_type& y)
    {
        return x == y;
    }
};

template<typename t, typename Enable>
struct value_traits : default_value_traits<t> {};

template<>
struct value_traits<double> : default_value_traits<double>
{
    static bool is_equal(value_type x, value_type y)
    {
        if (x == y)
            return true;
        else
        {
            using I = std::int64_t;
            constexpr int K = 1000;

            value_type x_, y_;

            return I(std::round(std::modf(x, &x_) * K)) == I(std::round(std::modf(y, &y_) * K)) &&
                   I(std::round(x_)) == I(std::round(y_));
        }
    }
};

template<> struct value_traits<bool> : default_value_traits<bool, int>
{
    static stored_type storage_from_qvariant(const QVariant& x)
    {
        if (x.type() == QVariant::String)
            return x.toBool();
        else
            return !!x.toInt();
    }

    static QVariant qvariant_from_storage(const stored_type& val)
    {
        return QVariant::fromValue<int>(!!val);
    }

    static value_type value_from_storage(const stored_type& x)
    {
        return !!x;
    }
};

template<> struct value_traits<float> : value_traits<float, double>
{
    static constexpr inline value_type pass_value(const value_type& x) { return x; }
};

template<>
struct value_traits<slider_value> : default_value_traits<slider_value>
{
    static slider_value value_with_default(const slider_value& val, const slider_value& def)
    {
        return { val.cur(), def.min(), def.max() };
    }

    static bool is_equal(const slider_value& x, const slider_value& y)
    {
        using tr = value_traits<double>;
        return tr::is_equal(x.cur(), y.cur());
    }
};

// Qt uses int a lot in slots so use it for all enums
template<typename t>
struct value_traits<t, std::enable_if_t<std::is_enum_v<t>>> : default_value_traits<t, int> {};

} // ns options::detail

