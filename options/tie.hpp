/* Copyright (c) 2015-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "export.hpp"
#include "value.hpp"
#include "compat/run-in-thread.hpp"
#include "compat/spinbox64.hpp"
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>

#include <cmath>

#if defined __GNUG__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wattributes"
#endif

namespace options {

template<typename t>
std::enable_if_t<std::is_enum_v<t>> tie_setting(value<t>& v, QComboBox* cb)
{
    cb->setCurrentIndex(cb->findData(int(static_cast<t>(v))));
    v = static_cast<t>(cb->currentData().toInt());

    value_::connect(cb, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                    &v, [&v, cb](int idx) { v = static_cast<t>(cb->itemData(idx).toInt()); },
                    v.DIRECT_CONNTYPE);

    value_::connect(&v, value_::value_changed<int>(),
                    cb, [cb](int x) { cb->setCurrentIndex(cb->findData(x)); },
                    v.SAFE_CONNTYPE);
}

template<typename t, typename From, typename To>
void tie_setting(value<t>& v, QComboBox* cb, From&& fn_to_index, To&& fn_to_value)
{
    cb->setCurrentIndex(fn_to_index(v));
    v = fn_to_value(cb->currentIndex(), cb->currentData());

    value_::connect(cb, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                    &v, [&v, cb, fn_to_value](int idx) { v = fn_to_value(idx, cb->currentData()); },
                    v.DIRECT_CONNTYPE);
    value_::connect(&v, value_::value_changed<t>(),
                    cb, [cb, fn_to_index](detail::cv_qualified<t>& v) { cb->setCurrentIndex(fn_to_index(v)); },
                    v.SAFE_CONNTYPE);
}

template<typename t, typename F>
void tie_setting(value<t>& v, QLabel* lb, F&& fun)
{
    auto closure = [lb, fun](detail::cv_qualified<t> v) { lb->setText(fun(v)); };

    closure(v());
    value_::connect(&v, value_::value_changed<t>(),
                    lb, closure,
                    v.SAFE_CONNTYPE);
}

template<typename t, typename F>
void tie_setting(value<t>& v, QObject* obj, F&& fun)
{
    if (obj == nullptr)
        abort();

    fun(v());

    value_::connect(&v, value_::value_changed<t>(),
                    obj, fun,
                    v.DIRECT_CONNTYPE);
}

OTR_OPTIONS_EXPORT void tie_setting(value<int>& v, QComboBox* cb);
OTR_OPTIONS_EXPORT void tie_setting(value<QString>& v, QComboBox* cb);
OTR_OPTIONS_EXPORT void tie_setting(value<QVariant>& v, QComboBox* cb);
OTR_OPTIONS_EXPORT void tie_setting(value<bool>& v, QCheckBox* cb);
OTR_OPTIONS_EXPORT void tie_setting(value<bool>& v, QRadioButton* cb);
OTR_OPTIONS_EXPORT void tie_setting(value<double>& v, QDoubleSpinBox* dsb);
OTR_OPTIONS_EXPORT void tie_setting(value<int>& v, QSpinBox* sb);
OTR_OPTIONS_EXPORT void tie_setting(value<long long>& v, spinbox64* sb);
OTR_OPTIONS_EXPORT void tie_setting(value<QString>& v, QLineEdit* le);
OTR_OPTIONS_EXPORT void tie_setting(value<QString>& v, QLabel* lb);
OTR_OPTIONS_EXPORT void tie_setting(value<int>& v, QTabWidget* t);
OTR_OPTIONS_EXPORT void tie_setting(value<slider_value>& v, QSlider* w);

} // ns options

#if defined __GNUG__
#   pragma GCC diagnostic pop
#endif
