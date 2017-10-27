#include "export.hpp"

#include "compat/functional.hpp"
#include "slider.hpp"

#include <QString>

#include <type_traits>

namespace options {
namespace detail {

template<typename t, typename u = t, typename Enable = void>
struct default_value_traits
{
    using element_type = std::decay_t<t>;
    using value_type = std::decay_t<u>;

    static inline value_type from_value(const value_type& val, const value_type&) { return val; }
    static inline value_type from_storage(const element_type& x) { return static_cast<value_type>(x); }
    static inline element_type to_storage(const value_type& val) { return static_cast<element_type>(val); }
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
        return slider_value(val.cur(), def.min(), def.max());
    }
};

// Qt uses int a lot in slots so use it for all enums
template<typename t>
struct value_traits<t, t, std::enable_if_t<is_enum_v<t>>> : public default_value_traits<int, t>
{
};

template<>
struct value_traits<float> : public default_value_traits<float, double, void>
{
};

} // ns detail
} // ns options
