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

namespace options {

template<typename t>
typename std::enable_if<std::is_enum<t>::value>::type
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
    base_value::connect(&v, static_cast<void (base_value::*)(int) const>(&base_value::valueChanged),
                        cb, [cb](int x) { cb->setCurrentIndex(cb->findData(x)); },
                        v.SAFE_CONNTYPE);
}

template<typename t, typename... xs>
void tie_setting(value<t>& v, QLabel* lb, const QString& format, const xs&... args)
{
    auto closure = [=](const t& x) { lb->setText(format.arg(x, args...)); };

    closure(v());
    base_value::connect(&v, static_cast<void(base_value::*)(const t&) const>(&base_value::valueChanged),
                        lb, closure,
                        v.SAFE_CONNTYPE);
}

template<typename t, typename F, typename... xs>
decltype((void)((std::declval<F>())(std::declval<const t&>())))
tie_setting(value<t>& v, QLabel* lb, F&& fun, const QString& fmt, const xs&... args)
{
    auto closure = [=](const t& x) { lb->setText(fmt.arg(fun(x), args...)); };

    closure(v());
    base_value::connect(&v, static_cast<void(base_value::*)(const t&) const>(&base_value::valueChanged),
                        lb, closure,
                        v.SAFE_CONNTYPE);
}

template<typename t, typename F, typename... xs>
decltype((void)((std::declval<F>())(std::declval<const t&>())))
tie_setting(value<t>& v, QLabel* lb, F&& fun)
{
    tie_setting(v, lb, fun, QStringLiteral("%1"));
}

OTR_OPTIONS_EXPORT void tie_setting(value<int>& v, QComboBox* cb);
OTR_OPTIONS_EXPORT void tie_setting(value<QString>& v, QComboBox* cb);
OTR_OPTIONS_EXPORT void tie_setting(value<QVariant>& v, QComboBox* cb);
OTR_OPTIONS_EXPORT void tie_setting(value<bool>& v, QCheckBox* cb);
OTR_OPTIONS_EXPORT void tie_setting(value<double>& v, QDoubleSpinBox* dsb);
OTR_OPTIONS_EXPORT void tie_setting(value<int>& v, QSpinBox* sb);
OTR_OPTIONS_EXPORT void tie_setting(value<int>& v, QSlider* sl);
OTR_OPTIONS_EXPORT void tie_setting(value<QString>& v, QLineEdit* le);
OTR_OPTIONS_EXPORT void tie_setting(value<QString>& v, QLabel* lb);
OTR_OPTIONS_EXPORT void tie_setting(value<int>& v, QTabWidget* t);
OTR_OPTIONS_EXPORT void tie_setting(value<slider_value>& v, QSlider* w);

} // ns options
