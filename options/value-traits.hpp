#pragma once

#include "export.hpp"

#include "slider.hpp"

#include <QString>

#include <type_traits>

namespace options::detail {

template<typename t, typename u = t, typename Enable = void>
struct default_value_traits
{
    virtual ~default_value_traits() = default;

    using stored_type = std::decay_t<t>;
    using value_type = std::decay_t<u>;

    static value_type from_value(const value_type& val, const value_type&) { return val; }
    static value_type from_storage(const stored_type& x) { return static_cast<value_type>(x); }
    static stored_type to_storage(const value_type& val) { return static_cast<stored_type>(val); }

    static value_type value_from_variant(const QVariant& x)
    {
        return from_storage(storage_from_variant(x));
    }

    static stored_type storage_from_variant(const QVariant& x)
    {
        return x.value<stored_type>();
    }
};

template<typename t, typename u = t, typename Enable = void>
struct value_traits : default_value_traits<t, u, Enable>
{
};

template<>
struct value_traits<slider_value> : default_value_traits<slider_value>
{
    static inline slider_value from_value(const slider_value& val, const slider_value& def)
    {
        return { val.cur(), def.min(), def.max() };
    }
};

// Qt uses int a lot in slots so use it for all enums
template<typename t>
struct value_traits<t, t, std::enable_if_t<std::is_enum_v<t>>> : public default_value_traits<int, t>
{
};

template<>
struct value_traits<float> : public default_value_traits<float, double, void>
{
};

} // ns options::detail

