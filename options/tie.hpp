/* Copyright (c) 2015-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "export.hpp"
#include "value.hpp"

#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QSlider>
#include <QLineEdit>
#include <QLabel>
#include <QTabWidget>

#include <cmath>

#if defined __GNUG__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wattributes"
#endif

namespace options {

template<typename t>
std::enable_if_t<std::is_enum<t>::value>
tie_setting(value<t>& v, QComboBox* cb)
{
    cb->setCurrentIndex(cb->findData(int(static_cast<t>(v))));
    v = static_cast<t>(cb->currentData().toInt());

    base_value::connect(cb,
                        static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                        &v,
                        [&v, cb](int idx)
                        {
                            run_in_thread_sync(cb,
                                               [&]() {
                                                    v = static_cast<t>(cb->itemData(idx).toInt());
                                               });
                        },
                        v.DIRECT_CONNTYPE);
    base_value::connect(&v, base_value::value_changed<int>(),
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
