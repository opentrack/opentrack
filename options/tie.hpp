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

    // QObject::connect plays badly with std::bind of std::shared_ptr. Data seems to get freed.
    // Direct accesses of cb->currentData within arbitrary thread context cause crashes as well.
    // Hence we go for a verbose implementation.

    std::vector<int> enum_cases;
    enum_cases.reserve(unsigned(cb->count()));

    for (int i = 0; i < cb->count(); i++)
        enum_cases.push_back(cb->itemData(i).toInt());

    struct fn1
    {
        value<t>& v;
        QComboBox* cb;
        std::vector<int> enum_cases;

        fn1(value<t>& v, QComboBox* cb, const std::vector<int>& enum_cases) :
            v(v),
            cb(cb),
            enum_cases(enum_cases)
        {
        }

        void operator()(int idx)
        {
            if (idx < 0 || idx >= (int)enum_cases.size())
                v = static_cast<t>(-1);
            else
                v = static_cast<t>(t(std::intptr_t(enum_cases[idx])));
        }
    };

    struct fn2
    {
        value<t>& v;
        QComboBox* cb;
        std::vector<int> enum_cases;

        fn2(value<t>& v, QComboBox* cb, const std::vector<int>& enum_cases) :
            v(v),
            cb(cb),
            enum_cases(enum_cases)
        {
        }

        void operator()(int val)
        {
            for (unsigned i = 0; i < enum_cases.size(); i++)
            {
                if (val == enum_cases[i])
                {
                    cb->setCurrentIndex(i);
                    return;
                }
            }
            cb->setCurrentIndex(-1);
        }
    };

    base_value::connect(cb,
                        static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                        &v,
                        fn1(v, cb, enum_cases),
                        v.DIRECT_CONNTYPE);
    base_value::connect(&v,
                        static_cast<void (base_value::*)(int) const>(&base_value::valueChanged),
                        cb,
                        fn2(v, cb, enum_cases),
                        v.DIRECT_CONNTYPE);
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

    {
        const int q_diff = q_max - q_min;
        slider_value sv(v);
        const double sv_c = sv.max() - sv.min();

        w->setValue(int((sv.cur() - sv.min()) / sv_c * q_diff + q_min));
        v = slider_value(q_diff <= 0 ? 0 : (w->value() - q_min) * sv_c / (double)q_diff + sv.min(), sv.min(), sv.max());
    }

    base_value::connect(w,
                        &QSlider::valueChanged,
                        &v,
                        [=, &v](int pos) { v = v->update_from_slider(pos, q_min, q_max); },
                        v.DIRECT_CONNTYPE);
    base_value::connect(&v,
                        static_cast<void(base_value::*)(double) const>(&base_value::valueChanged),
                        w,
                        [=, &v](double) { w->setValue(v->to_slider_pos(q_min, q_max)); },
                        v.SAFE_CONNTYPE);
}

}
