/* Copyright (c) 2015-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "value-traits.hpp"
#include "value.hpp"
#include "compat/run-in-thread.hpp"
#include "compat/macros.hpp"

#include <type_traits>

#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QSlider>
#include <QLineEdit>
#include <QLabel>
#include <QTabWidget>

#include <cmath>

#include "export.hpp"

#if defined __GNUG__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wattributes"
#endif

namespace options {

namespace detail {

template<typename t>
struct tie_setting_traits_helper
{
    using traits = detail::value_traits<t>;
    using value_type = typename traits::value_type;
    using element_type = typename traits::element_type;

    static element_type to_element_type(const value<t>& v)
    {
        return static_cast<element_type>(static_cast<value_type>(v));
    }
};

template<typename t, typename Enable = void>
struct tie_setting_traits final : tie_setting_traits_helper<t>
{
    static constexpr inline bool should_bind_to_itemdata() { return false; }
};

template<typename t>
struct tie_setting_traits<t, std::enable_if_t<std::is_enum_v<t>>> : tie_setting_traits_helper<t>
{
    static constexpr inline bool should_bind_to_itemdata() { return true; }

    static t itemdata_to_value(int, const QVariant& var)
    {
        return static_cast<t>(var.toInt());
    }
};

} // ns options::details

template<typename t, typename traits_type = detail::tie_setting_traits<t>>
std::enable_if_t<traits_type::should_bind_to_itemdata()>
tie_setting(value<t>& v, QComboBox* cb, const traits_type& traits = traits_type())
{
    using element_type = typename detail::value_traits<t>::element_type;

    cb->setCurrentIndex(cb->findData(traits.to_element_type(v)));
    v = traits.itemdata_to_value(cb->currentIndex(), cb->currentData());

    base_value::connect(cb,
                        static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                        &v,
                        [&v, cb, traits](int idx)
                        {
                            run_in_thread_sync(cb,
                                               [&, traits]() {
                                                    v = traits.itemdata_to_value(idx, cb->currentData());
                                               });
                        },
                        v.DIRECT_CONNTYPE);
    base_value::connect(&v, base_value::value_changed<element_type>(),
                        cb, [cb](int x) {
                            run_in_thread_sync(cb, [&]() { cb->setCurrentIndex(cb->findData(x)); });
                        },
                        v.DIRECT_CONNTYPE);
}

template<typename t, typename F>
void tie_setting(value<t>& v, QLabel* lb, F&& fun)
{
    auto closure = [=](cv_qualified<t> x) { lb->setText(fun(x)); };

    closure(v());
    base_value::connect(&v, base_value::value_changed<t>(),
                        lb, closure,
                        v.SAFE_CONNTYPE);
}

template<typename t, typename F>
void tie_setting(value<t>& v, QObject* obj, F&& fun)
{
    if (obj == nullptr)
        abort();

    fun(v());

    base_value::connect(&v, base_value::value_changed<t>(),
                        obj, fun,
                        v.DIRECT_CONNTYPE);
}

// XXX TODO add combobox transform both ways via std::function
// need for non-translated `module_settings' dylib names

OTR_OPTIONS_EXPORT void tie_setting(value<int>& v, QComboBox* cb);
OTR_OPTIONS_EXPORT void tie_setting(value<QString>& v, QComboBox* cb);
OTR_OPTIONS_EXPORT void tie_setting(value<QVariant>& v, QComboBox* cb);
OTR_OPTIONS_EXPORT void tie_setting(value<bool>& v, QCheckBox* cb);
OTR_OPTIONS_EXPORT void tie_setting(value<double>& v, QDoubleSpinBox* dsb);
OTR_OPTIONS_EXPORT void tie_setting(value<int>& v, QSpinBox* sb);
OTR_OPTIONS_EXPORT void tie_setting(value<QString>& v, QLineEdit* le);
OTR_OPTIONS_EXPORT void tie_setting(value<QString>& v, QLabel* lb);
OTR_OPTIONS_EXPORT void tie_setting(value<int>& v, QTabWidget* t);
OTR_OPTIONS_EXPORT void tie_setting(value<slider_value>& v, QSlider* w);

} // ns options

#if defined __GNUG__
#   pragma GCC diagnostic pop
#endif
