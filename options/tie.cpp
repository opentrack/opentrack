/* Copyright (c) 2015-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "tie.hpp"

namespace options {

OTR_OPTIONS_EXPORT void tie_setting(value<int>& v, QComboBox* cb)
{
    cb->setCurrentIndex(v);
    v = cb->currentIndex();
    base_value::connect(cb, SIGNAL(currentIndexChanged(int)), &v, SLOT(setValue(int)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(int)), cb, SLOT(setCurrentIndex(int)), v.SAFE_CONNTYPE);
}

OTR_OPTIONS_EXPORT void tie_setting(value<QString>& v, QComboBox* cb)
{
    cb->setCurrentText(v);
    v = cb->currentText();
    base_value::connect(cb, SIGNAL(currentTextChanged(QString)), &v, SLOT(setValue(const QString&)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(const QString&)), cb, SLOT(setCurrentText(const QString&)), v.SAFE_CONNTYPE);
}

OTR_OPTIONS_EXPORT void tie_setting(value<QVariant>& v, QComboBox* cb)
{
    auto set_idx = [cb](const QVariant& var) {
        const int sz = cb->count();
        int idx = -1;

        for (int k = 0; k < sz; k++)
        {
            if (cb->itemData(k) == var)
            {
                idx = k;
                break;
            }
        }
        cb->setCurrentIndex(idx);
        return idx;
    };

    const int idx = set_idx(v);

    if (idx != -1)
        v = cb->itemData(idx);
    else
        v = QVariant(QVariant::Invalid);

    base_value::connect(cb, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                        &v, [cb, &v](int idx) {
        v = cb->itemData(idx);
    }, v.DIRECT_CONNTYPE);
    base_value::connect(&v, base_value::value_changed<QVariant>(),
                        cb,
                        [cb, set_idx](const QVariant& var) {
        run_in_thread_sync(cb, [&]() {
            set_idx(var);
        });
    }, v.DIRECT_CONNTYPE);
}

// XXX TODO need variant with setEnabled based on lambda retval -- sh 20170524

OTR_OPTIONS_EXPORT void tie_setting(value<bool>& v, QCheckBox* cb)
{
    cb->setChecked(v);
    base_value::connect(cb, SIGNAL(toggled(bool)), &v, SLOT(setValue(bool)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(bool)), cb, SLOT(setChecked(bool)), v.SAFE_CONNTYPE);
}

OTR_OPTIONS_EXPORT void tie_setting(value<double>& v, QDoubleSpinBox* dsb)
{
    dsb->setValue(v);
    base_value::connect(dsb, SIGNAL(valueChanged(double)), &v, SLOT(setValue(double)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(double)), dsb, SLOT(setValue(double)), v.SAFE_CONNTYPE);
}

OTR_OPTIONS_EXPORT void tie_setting(value<int>& v, QSpinBox* sb)
{
    sb->setValue(v);
    base_value::connect(sb, SIGNAL(valueChanged(int)), &v, SLOT(setValue(int)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(int)), sb, SLOT(setValue(int)), v.SAFE_CONNTYPE);
}

OTR_OPTIONS_EXPORT void tie_setting(value<QString>& v, QLineEdit* le)
{
    le->setText(v);
    base_value::connect(le, SIGNAL(textChanged(QString)), &v, SLOT(setValue(QString)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, base_value::value_changed<QString>(), le, &QLineEdit::setText, v.SAFE_CONNTYPE);
}

OTR_OPTIONS_EXPORT void tie_setting(value<QString>& v, QLabel* lb)
{
    lb->setText(v);
    base_value::connect(&v, base_value::value_changed<QString>(), lb, &QLabel::setText, v.SAFE_CONNTYPE);
}

OTR_OPTIONS_EXPORT void tie_setting(value<int>& v, QTabWidget* t)
{
    t->setCurrentIndex(v);
    base_value::connect(t, SIGNAL(currentChanged(int)), &v, SLOT(setValue(int)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(int)), t, SLOT(setCurrentIndex(int)), v.SAFE_CONNTYPE);
}

OTR_OPTIONS_EXPORT void tie_setting(value<slider_value>& v, QSlider* w)
{
    {
        const int q_min = w->minimum();
        const int q_max = w->maximum();

        w->setValue(v().to_slider_pos(q_min, q_max));
        v = v().update_from_slider(w->value(), q_min, q_max);
    }

    base_value::connect(w,
                        &QSlider::valueChanged,
                        &v,
                        [=, &v](int pos)
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

    base_value::connect(&v,
                        base_value::value_changed<slider_value>(),
                        w,
                        [=, &v](double) {
        run_in_thread_sync(w, [=, &v]()
        {
            const int q_min = w->minimum();
            const int q_max = w->maximum();
            const int pos = v().to_slider_pos(q_min, q_max);
            w->setValue(pos);
            v = v().update_from_slider(w->value(), q_min, q_max);
        });
    },
    v.DIRECT_CONNTYPE);
}

} // ns options
