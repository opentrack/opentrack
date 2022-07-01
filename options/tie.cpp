/* Copyright (c) 2015-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "tie.hpp"
#include "compat/run-in-thread.hpp"

#include "value-traits.hpp"

#include <cmath>

namespace options {

void tie_setting(value<int>& v, QComboBox* cb)
{
    cb->setCurrentIndex(v);
    v = cb->currentIndex();
    v.connect_to(cb, &QComboBox::setCurrentIndex);
    v.connect_from(cb, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged));
}

void tie_setting(value<QString>& v, QComboBox* cb)
{
    cb->setCurrentText(v);
    v = cb->currentText();
    v.connect_to(cb, &QComboBox::currentTextChanged);
    v.connect_from(cb, &QComboBox::setCurrentText);
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

    v.connect_from(cb, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                   [cb, &v](int idx) { v = cb->itemData(idx); });
    v.connect_to(cb, [fn = std::move(set_idx)](const QVariant& var) { fn(var); });
}

void tie_setting(value<bool>& v, QRadioButton* cb)
{
    cb->setChecked(v);
    v.connect_to(cb, &QRadioButton::setChecked);
    v.connect_from(cb, &QRadioButton::toggled);
}

void tie_setting(value<bool>& v, QCheckBox* cb)
{
    cb->setChecked(v);
    v.connect_to(cb, &QCheckBox::setChecked);
    v.connect_from(cb, &QCheckBox::toggled);
}

void tie_setting(value<double>& v, QDoubleSpinBox* dsb)
{
    dsb->setValue(v);
    v.connect_to(dsb, &QDoubleSpinBox::setValue);
    v.connect_from(dsb, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged));
}

void tie_setting(value<int>& v, QSpinBox* sb)
{
    sb->setValue(v);
    v.connect_to(sb, &QSpinBox::setValue);
    v.connect_from(sb, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged));
}

void tie_setting(value<QString>& v, QLineEdit* le)
{
    le->setText(v);
    v.connect_to(le, &QLineEdit::setText);
    v.connect_from(le, &QLineEdit::textChanged);
}

void tie_setting(value<QString>& v, QLabel* lb)
{
    lb->setText(v);
    v.connect_to(lb, &QLabel::setText);
}

void tie_setting(value<int>& v, QTabWidget* t)
{
    t->setCurrentIndex(v);
    v.connect_to(t, &QTabWidget::setCurrentIndex);
    v.connect_from(t, &QTabWidget::currentChanged);
}

void tie_setting(value<slider_value>& v, QSlider* w)
{
    {
        const int q_min = w->minimum();
        const int q_max = w->maximum();

        w->setValue(v().to_slider_pos(q_min, q_max));
        v = v().update_from_slider(w->value(), q_min, q_max);
    }

    v.connect_from(w, &QSlider::valueChanged, [=, &v](int pos) {
        run_in_thread_sync(w, [&]() {
            const int q_min = w->minimum();
            const int q_max = w->maximum();
            v = v().update_from_slider(pos, q_min, q_max);
            w->setValue(v().to_slider_pos(q_min, q_max));
        });
    }, v.DIRECT_CONNTYPE);

    v.connect_to(w, [=, &v](double) {
        run_in_thread_sync(w, [=, &v]() {
            const int q_min = w->minimum();
            const int q_max = w->maximum();
            const int pos = v->to_slider_pos(q_min, q_max);
            v = v->update_from_slider(pos, q_min, q_max);
            w->setValue(pos);
        });
    }, v.DIRECT_CONNTYPE);
}

} // ns options
