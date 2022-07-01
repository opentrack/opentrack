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

#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QSlider>
#include <QLineEdit>
#include <QLabel>
#include <QTabWidget>
#include <QRadioButton>

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

    v.connect_from(cb, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                   [&v, cb](int idx) { v = static_cast<t>(cb->itemData(idx).toInt()); });
    v.connect_to(cb, [cb](int x) { cb->setCurrentIndex(cb->findData(x)); });
}

template<typename t, typename From, typename To>
void tie_setting(value<t>& v, QComboBox* cb, From&& fn_to_index, To&& fn_to_value)
{
    cb->setCurrentIndex(fn_to_index(v));
    v = fn_to_value(cb->currentIndex(), cb->currentData());

    v.connect_from(cb, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                   [&v, cb, fn = std::forward<To>(fn_to_value)](int idx) { v = fn(idx, cb->currentData()); });
    v.connect_to(cb, [cb, fn = std::forward<From>(fn_to_index)](detail::cv_qualified<t> v) { cb->setCurrentIndex(fn(v)); });
}

template<typename t, typename F>
void tie_setting(value<t>& v, QLabel* lb, F&& fun)
{
    auto closure = [lb, fn = std::forward<F>(fun)](detail::cv_qualified<t> v) {
        lb->setText(fn(v));
    };

    closure(v());
    v.connect_to(lb, std::move(closure));
}

template<typename t, typename F>
void tie_setting(value<t>& v, QObject* obj, F&& fun)
{
    if (obj == nullptr)
        abort();

    fun(v());
    v.connect_to(obj, std::forward<F>(fun));
}

OTR_OPTIONS_EXPORT void tie_setting(value<int>& v, QComboBox* cb);
OTR_OPTIONS_EXPORT void tie_setting(value<QString>& v, QComboBox* cb);
OTR_OPTIONS_EXPORT void tie_setting(value<QVariant>& v, QComboBox* cb);
OTR_OPTIONS_EXPORT void tie_setting(value<bool>& v, QCheckBox* cb);
OTR_OPTIONS_EXPORT void tie_setting(value<bool>& v, QRadioButton* cb);
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
