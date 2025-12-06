/* Copyright (c) 2015-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "tie.hpp"
#include "compat/run-in-thread.hpp"
#include "compat/macros.h"

#include "value-traits.hpp"

#include <cmath>

namespace options {

void tie_setting(value<int>& v, QComboBox* cb)
{
    cb->setCurrentIndex(v);
    v = cb->currentIndex();
    value_::connect(cb, SIGNAL(currentIndexChanged(int)), &v, SLOT(setValue(int)), v.DIRECT_CONNTYPE);
    value_::connect(&v, SIGNAL(valueChanged(int)), cb, SLOT(setCurrentIndex(int)), v.SAFE_CONNTYPE);
}

void tie_setting(value<QString>& v, QComboBox* cb)
{
    cb->setCurrentText(v);
    v = cb->currentText();
    auto set_current_text = [cb, &v](const QString& str) {
        cb->setCurrentText(str);
        v = cb->currentText();
    };
    value_::connect(cb, &QComboBox::currentTextChanged, &v, v.value_changed<QString>(), v.DIRECT_CONNTYPE);
    value_::connect(&v, v.value_changed<QString>(), cb, set_current_text, v.SAFE_CONNTYPE);
}

void tie_setting(value<QVariant>& v, QComboBox* cb)
{
    auto set_idx = [cb](const QVariant& var) {
        const int sz = cb->count();
        int idx = -1;

        for (int k = 0; k < sz; k++)
            if (cb->itemData(k) == var) {
                idx = k;
                break;
            }
        cb->setCurrentIndex(idx);
        return idx;
    };

    const int idx = set_idx(v);

    if (idx != -1)
        v = cb->itemData(idx);
    else
        v = {};

    value_::connect(cb, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                    &v, [cb, &v](int idx) { v = cb->itemData(idx); },
                    v.DIRECT_CONNTYPE);
    value_::connect(&v, value_::value_changed<QVariant>(),
                    cb, [set_idx](const QVariant& var) { set_idx(var); },
                    v.SAFE_CONNTYPE);
}

void tie_setting(value<bool>& v, QRadioButton* cb)
{
    cb->setChecked(v);
    value_::connect(cb, SIGNAL(toggled(bool)), &v, SLOT(setValue(bool)), v.DIRECT_CONNTYPE);
    value_::connect(&v, SIGNAL(valueChanged(bool)), cb, SLOT(setChecked(bool)), v.SAFE_CONNTYPE);
}

void tie_setting(value<bool>& v, QCheckBox* cb)
{
    cb->setChecked(v);
    value_::connect(cb, SIGNAL(toggled(bool)), &v, SLOT(setValue(bool)), v.DIRECT_CONNTYPE);
    value_::connect(&v, SIGNAL(valueChanged(bool)), cb, SLOT(setChecked(bool)), v.SAFE_CONNTYPE);
}

void tie_setting(value<double>& v, QDoubleSpinBox* dsb)
{
    dsb->setValue(v);
    value_::connect(dsb, SIGNAL(valueChanged(double)), &v, SLOT(setValue(double)), v.DIRECT_CONNTYPE);
    value_::connect(&v, SIGNAL(valueChanged(double)), dsb, SLOT(setValue(double)), v.SAFE_CONNTYPE);
}

void tie_setting(value<int>& v, QSpinBox* sb)
{
    sb->setValue(v);
    value_::connect(sb, SIGNAL(valueChanged(int)), &v, SLOT(setValue(int)), v.DIRECT_CONNTYPE);
    value_::connect(&v, SIGNAL(valueChanged(int)), sb, SLOT(setValue(int)), v.SAFE_CONNTYPE);
}

void tie_setting(value<long long>& v, QLongLongSpinBox* sb)
{
    sb->setValue(v);
    value_::connect(sb, SIGNAL(valueChanged(long long)), &v, SLOT(setValue(long long)), v.DIRECT_CONNTYPE);
    value_::connect(&v, SIGNAL(valueChanged(long long)), sb, SLOT(setValue(long long)), v.SAFE_CONNTYPE);
}


void tie_setting(value<QString>& v, QLineEdit* le)
{
    le->setText(v);
    value_::connect(le, SIGNAL(textChanged(QString)), &v, SLOT(setValue(QString)), v.DIRECT_CONNTYPE);
    value_::connect(&v, value_::value_changed<QString>(), le, &QLineEdit::setText, v.SAFE_CONNTYPE);
}

void tie_setting(value<QString>& v, QLabel* lb)
{
    lb->setText(v);
    value_::connect(&v, value_::value_changed<QString>(), lb, &QLabel::setText, v.SAFE_CONNTYPE);
}

void tie_setting(value<int>& v, QTabWidget* t)
{
    t->setCurrentIndex(v);
    value_::connect(t, SIGNAL(currentChanged(int)), &v, SLOT(setValue(int)), v.DIRECT_CONNTYPE);
    value_::connect(&v, SIGNAL(valueChanged(int)), t, SLOT(setCurrentIndex(int)), v.SAFE_CONNTYPE);
}

void tie_setting(value<slider_value>& v, QSlider* w)
{
    {
        const int q_min = w->minimum();
        const int q_max = w->maximum();

        w->setValue(v().to_slider_pos(q_min, q_max));
        v = v().update_from_slider(w->value(), q_min, q_max);
    }

    value_::connect(w, &QSlider::valueChanged, &v, [=, &v](int pos)
    {
        run_in_thread_sync(w, [&]()
        {
            const int q_min = w->minimum();
            const int q_max = w->maximum();
            v = v().update_from_slider(pos, q_min, q_max);
            w->setValue(v().to_slider_pos(q_min, q_max));
        });
    },
    v.DIRECT_CONNTYPE);

    value_::connect(&v,
                    value_::value_changed<slider_value>(),
                    w,
                    [=, &v](double) {
        run_in_thread_sync(w, [=, &v]()
        {
            const int q_min = w->minimum();
            const int q_max = w->maximum();
            const int pos = v->to_slider_pos(q_min, q_max);
            v = v->update_from_slider(pos, q_min, q_max);
            w->setValue(pos);
        });
    },
    v.DIRECT_CONNTYPE);
}

} // ns options
