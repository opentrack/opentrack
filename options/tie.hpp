/* Copyright (c) 2015-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

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

template<typename t, typename q>
inline void tie_setting(value<t>&, q*);

template<typename t>
inline
typename std::enable_if<std::is_enum<t>::value>::type
tie_setting(value<t>& v, QComboBox* cb)
{
    cb->setCurrentIndex(cb->findData((unsigned)static_cast<t>(v)));
    v = static_cast<t>(cb->currentData().toInt());

    std::vector<int> enum_cases(unsigned(cb->count()));

    for (int i = 0; i < cb->count(); i++)
        enum_cases[i] = cb->itemData(i).toInt();

    base_value::connect(cb,
                        static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                        &v,
                        [&, enum_cases](int idx) {
                            if (idx < 0 || idx >= (int)enum_cases.size())
                                v = static_cast<t>(-1);
                            else
                                v = static_cast<t>(enum_cases[idx]);
                        },
                        v.DIRECT_CONNTYPE);
    base_value::connect(&v,
                        static_cast<void (base_value::*)(int) const>(&base_value::valueChanged),
                        cb,
                        [&, enum_cases](int val) {
                            for (unsigned i = 0; i < enum_cases.size(); i++)
                            {
                                if (val == enum_cases[i])
                                {
                                    cb->setCurrentIndex(i);
                                    return;
                                }
                            }
                            cb->setCurrentIndex(-1);
                        },
                        // don't change or else hatire crashes -sh 20160917
                        Qt::QueuedConnection);
}

template<>
inline void tie_setting(value<int>& v, QComboBox* cb)
{
    cb->setCurrentIndex(v);
    v = cb->currentIndex();
    base_value::connect(cb, SIGNAL(currentIndexChanged(int)), &v, SLOT(setValue(int)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(int)), cb, SLOT(setCurrentIndex(int)), v.SAFE_CONNTYPE);
}

template<>
inline void tie_setting(value<QString>& v, QComboBox* cb)
{
    cb->setCurrentText(v);
    v = cb->currentText();
    base_value::connect(cb, SIGNAL(currentTextChanged(QString)), &v, SLOT(setValue(QString)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(QString)), cb, SLOT(setCurrentText(QString)), v.SAFE_CONNTYPE);
}

template<>
inline void tie_setting(value<bool>& v, QCheckBox* cb)
{
    cb->setChecked(v);
    base_value::connect(cb, SIGNAL(toggled(bool)), &v, SLOT(setValue(bool)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(bool)), cb, SLOT(setChecked(bool)), v.SAFE_CONNTYPE);
}

template<>
inline void tie_setting(value<double>& v, QDoubleSpinBox* dsb)
{
    dsb->setValue(v);
    base_value::connect(dsb, SIGNAL(valueChanged(double)), &v, SLOT(setValue(double)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(double)), dsb, SLOT(setValue(double)), v.SAFE_CONNTYPE);
}

template<>
inline void tie_setting(value<int>& v, QSpinBox* sb)
{
    sb->setValue(v);
    base_value::connect(sb, SIGNAL(valueChanged(int)), &v, SLOT(setValue(int)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(int)), sb, SLOT(setValue(int)), v.SAFE_CONNTYPE);
}

template<>
inline void tie_setting(value<int>& v, QSlider* sl)
{
    sl->setValue(v);
    v = sl->value();
    base_value::connect(sl, SIGNAL(valueChanged(int)), &v, SLOT(setValue(int)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(int)), sl, SLOT(setValue(int)), v.SAFE_CONNTYPE);
}

template<>
inline void tie_setting(value<QString>& v, QLineEdit* le)
{
    le->setText(v);
    base_value::connect(le, SIGNAL(textChanged(QString)), &v, SLOT(setValue(QString)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(QString)),le, SLOT(setText(QString)), v.SAFE_CONNTYPE);
}

template<>
inline void tie_setting(value<QString>& v, QLabel* lb)
{
    lb->setText(v);
    base_value::connect(&v, SIGNAL(valueChanged(QString)), lb, SLOT(setText(QString)), v.DIRECT_CONNTYPE);
}

template<>
inline void tie_setting(value<int>& v, QTabWidget* t)
{
    t->setCurrentIndex(v);
    base_value::connect(t, SIGNAL(currentChanged(int)), &v, SLOT(setValue(int)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(int)), t, SLOT(setCurrentIndex(int)), v.SAFE_CONNTYPE);
}

template<>
inline void tie_setting(value<slider_value>& v, QSlider* w)
{
    // we can't get these at runtime since signals cross threads
    const int q_min = w->minimum();
    const int q_max = w->maximum();

    w->setValue(v->to_slider_pos(q_min, q_max));
    v = v->update_from_slider(w->value(), q_min, q_max);

    base_value::connect(w,
                        &QSlider::valueChanged,
                        &v,
                        [=, &v](int pos) {
                            v = v->update_from_slider(pos, q_min, q_max);
                            w->setValue(v->to_slider_pos(q_min, q_max));
                        },
                        v.DIRECT_CONNTYPE);
    base_value::connect(&v,
                        static_cast<void(base_value::*)(double) const>(&base_value::valueChanged),
                        w,
                        [=, &v](double) {
                            w->setValue(v->to_slider_pos(q_min, q_max));
                            v = v->update_from_slider(w->value(), q_min, q_max);
                        },
                        v.SAFE_CONNTYPE);
}

}
