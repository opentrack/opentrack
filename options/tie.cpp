/* Copyright (c) 2015-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "tie.hpp"

namespace options {

OPENTRACK_OPTIONS_EXPORT void tie_setting(value<int>& v, QComboBox* cb)
{
    cb->setCurrentIndex(v);
    v = cb->currentIndex();
    base_value::connect(cb, SIGNAL(currentIndexChanged(int)), &v, SLOT(setValue(int)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(int)), cb, SLOT(setCurrentIndex(int)), v.SAFE_CONNTYPE);
}

OPENTRACK_OPTIONS_EXPORT void tie_setting(value<QString>& v, QComboBox* cb)
{
    cb->setCurrentText(v);
    v = cb->currentText();
    base_value::connect(cb, SIGNAL(currentTextChanged(QString)), &v, SLOT(setValue(QString)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(QString)), cb, SLOT(setCurrentText(QString)), v.SAFE_CONNTYPE);
}

OPENTRACK_OPTIONS_EXPORT void tie_setting(value<bool>& v, QCheckBox* cb)
{
    cb->setChecked(v);
    base_value::connect(cb, SIGNAL(toggled(bool)), &v, SLOT(setValue(bool)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(bool)), cb, SLOT(setChecked(bool)), v.SAFE_CONNTYPE);
}

OPENTRACK_OPTIONS_EXPORT void tie_setting(value<double>& v, QDoubleSpinBox* dsb)
{
    dsb->setValue(v);
    base_value::connect(dsb, SIGNAL(valueChanged(double)), &v, SLOT(setValue(double)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(double)), dsb, SLOT(setValue(double)), v.SAFE_CONNTYPE);
}

OPENTRACK_OPTIONS_EXPORT void tie_setting(value<int>& v, QSpinBox* sb)
{
    sb->setValue(v);
    base_value::connect(sb, SIGNAL(valueChanged(int)), &v, SLOT(setValue(int)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(int)), sb, SLOT(setValue(int)), v.SAFE_CONNTYPE);
}

OPENTRACK_OPTIONS_EXPORT void tie_setting(value<int>& v, QSlider* sl)
{
    sl->setValue(v);
    v = sl->value();
    base_value::connect(sl, SIGNAL(valueChanged(int)), &v, SLOT(setValue(int)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(int)), sl, SLOT(setValue(int)), v.SAFE_CONNTYPE);
}

OPENTRACK_OPTIONS_EXPORT void tie_setting(value<QString>& v, QLineEdit* le)
{
    le->setText(v);
    base_value::connect(le, SIGNAL(textChanged(QString)), &v, SLOT(setValue(QString)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(QString)),le, SLOT(setText(QString)), v.SAFE_CONNTYPE);
}

OPENTRACK_OPTIONS_EXPORT void tie_setting(value<QString>& v, QLabel* lb)
{
    lb->setText(v);
    base_value::connect(&v, SIGNAL(valueChanged(QString)), lb, SLOT(setText(QString)), v.DIRECT_CONNTYPE);
}

OPENTRACK_OPTIONS_EXPORT void tie_setting(value<int>& v, QTabWidget* t)
{
    t->setCurrentIndex(v);
    base_value::connect(t, SIGNAL(currentChanged(int)), &v, SLOT(setValue(int)), v.DIRECT_CONNTYPE);
    base_value::connect(&v, SIGNAL(valueChanged(int)), t, SLOT(setCurrentIndex(int)), v.SAFE_CONNTYPE);
}

OPENTRACK_OPTIONS_EXPORT void tie_setting(value<slider_value>& v, QSlider* w)
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
                        static_cast<void(base_value::*)(const slider_value&) const>(&base_value::valueChanged),
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
