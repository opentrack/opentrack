/* Copyright (c) 2013 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include <QSettings>
#include <QSettings>
#include <QMap>
#include <QString>
#include <QVariant>
#include <memory>
#include <cassert>
#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QSlider>

namespace options {
    template<typename T>
    inline T qcruft_to_t(const QVariant& t);

    template<>
    inline int qcruft_to_t<int>(const QVariant& t)
    {
        return t.toInt();
    }

    template<>
    inline bool qcruft_to_t<bool>(const QVariant& t)
    {
        return t.toBool();
    }

    template<>
    inline double qcruft_to_t<double>(const QVariant& t)
    {
        return t.toDouble();
    }

    template<>
    inline float qcruft_to_t<float>(const QVariant& t)
    {
        return t.toFloat();
    }

    // snapshot of qsettings group at given time
    class group {
    private:
        QMap<QString, QVariant> map;
        QString name;
    public:
        group(const QString& name, QSettings& s) : name(name)
        {
            s.beginGroup(name);
            for (auto& k : s.childKeys())
                map[k] = s.value(k);
            s.endGroup();
        }
        static constexpr const char* org = "opentrack";
        void save() {
            QSettings s(org);
            s.beginGroup(name);
            for (auto& k : map.keys())
                s.setValue(k, map[k]);
            s.endGroup();
        }
        template<typename T>
        T get(const QString& k) {
            return qcruft_to_t<T>(map.value(k));
        }

        void put(const QString& s, const QVariant& d)
        {
            map[s] = d;
        }
        bool contains(const QString& s)
        {
            return map.contains(s);
        }
    };

    class bundle {
    private:
        const QString group_name;
        group saved;
        group transient;
        bundle(const bundle&) = delete;
        bundle& operator=(const bundle&) = delete;
        bool modified;
    public:
        bundle(const QString& group_name, QSettings& s) :
            group_name(group_name),
            saved(group_name, s),
            transient(saved),
            modified(false)
        {
        }
        std::shared_ptr<bundle> make(const QString& name, QSettings& s) {
            assert(s.format() == QSettings::IniFormat);
            return std::make_shared<bundle>(name, s);
        }
        void store(const QString& name, QVariant& datum)
        {
            modified = true;
            transient.put(name, datum);
        }
        bool contains(const QString& name)
        {
            return transient.contains(name);
        }
        template<typename T>
        T get(QString& name) {
            transient.get<T>(name);
        }
        void save()
        {
            modified = false;
            saved = transient;
            transient.save();
        }
        void revert()
        {
            modified = false;
            transient = saved;
        }
    };

    typedef std::shared_ptr<bundle> pbundle;

    class QCruft : public QObject {
    };

    template<typename T>
    class value : public QCruft {
    private:
        QString self_name;
        pbundle b;
    public:
        value(const pbundle& b, const QString& name, T def) :
            self_name(name),
            b(b)
        {
            if (!b->contains(name))
            {
                QVariant cruft(def);
                b->store(self_name, cruft);
            }
        }
        operator T() { return b->get<T>(self_name); }
        T& operator=(const T& datum) {
            b->store(self_name, datum);
            emit valueChanged(datum);
            return datum;
        }
    public slots:
        void setValue(const T datum) {
            this->operator =(datum);
        }
    signals:
        void valueChanged(T datum);
    };

    template<typename T, typename Q>
    inline void tie(value<T>&, Q*);

    template<>
    inline void tie<int, QComboBox>(value<int>& v, QComboBox* cb)
    {
        QObject::connect(cb, SIGNAL(currentIndexChanged(int)), &v, SLOT(setValue(int)));
        QObject::connect(&v, SIGNAL(valueChanged(int)), &v, SLOT(setValue(int)));
    }

    template<>
    inline void tie<QString, QComboBox>(value<QString>& v, QComboBox* cb)
    {
        QObject::connect(cb, SIGNAL(currentTextChanged(QString)), &v, SLOT(setValue(QString)));
        QObject::connect(&v, SIGNAL(valueChanged(QString)), &v, SLOT(setValue(QString)));
    }

    template<>
    inline void tie<bool, QCheckBox>(value<bool>& v, QCheckBox* cb)
    {
        QObject::connect(cb, SIGNAL(toggled(bool)), &v, SLOT(setValue(bool)));
        QObject::connect(&v, SIGNAL(valueChanged(bool)), cb, SLOT(setChecked(bool)));
    }

    template<>
    inline void tie<double, QDoubleSpinBox>(value<double>& v, QDoubleSpinBox* dsb)
    {
        QObject::connect(dsb, SIGNAL(valueChanged(double)), &v, SLOT(setValue(double)));
        QObject::connect(&v, SIGNAL(valueChanged(double)), dsb, SLOT(setValue(double)));
    }

    template<>
    inline void tie<int, QSpinBox>(value<int>& v, QSpinBox* sb)
    {
        QObject::connect(sb, SIGNAL(valueChanged(int)), &v, SLOT(setValue(int)));
        QObject::connect(&v, SIGNAL(valueChanged(int)), sb, SLOT(setValue(int)));
    }

    template<>
    inline void tie<int, QSlider>(value<int>& v, QSlider* sl)
    {
        QObject::connect(sl, SIGNAL(valueChanged(int)), &v, SLOT(setValue(int)));
        QObject::connect(&v, SIGNAL(valueChanged(int)), sl, SLOT(setValue(int)));
    }
}
