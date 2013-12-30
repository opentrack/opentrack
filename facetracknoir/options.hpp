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
    inline QVariant qcruft_to_t<QVariant>(const QVariant& t)
    {
        return t;
    }

    // snapshot of qsettings group at given time
    class group {
    private:
        QMap<QString, QVariant> map;
        QString name;
    public:
        group(const QString& name) : name(name)
        {
            QSettings settings(group::org);
            QString currentFile =
                    settings.value("SettingsFile",
                                   QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
            QSettings iniFile(currentFile, QSettings::IniFormat);
            iniFile.beginGroup(name);
            for (auto& k : iniFile.childKeys())
                map[k] = iniFile.value(k);
            iniFile.endGroup();
        }
        static constexpr const char* org = "opentrack";
        void save() {
            QSettings settings(group::org);
            QString currentFile =
                    settings.value("SettingsFile",
                                   QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
            QSettings s(currentFile, QSettings::IniFormat);
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

    class impl_bundle {
    private:
        const QString group_name;
        group saved;
        group transient;
        impl_bundle(const impl_bundle&) = delete;
        impl_bundle& operator=(const impl_bundle&) = delete;
        bool modified;
    public:
        impl_bundle(const QString& group_name) :
            group_name(group_name),
            saved(group_name),
            transient(saved),
            modified(false)
        {
        }
        /* keep in mind doesn't fire signals */
        void reload() {
            saved = group(group_name);
            transient = saved;
        }

        std::shared_ptr<impl_bundle> make(const QString& name) {
            return std::make_shared<impl_bundle>(name);
        }
        void store(const QString& name, const QVariant& datum)
        {
            if (!transient.contains(name) || datum != transient.get<QVariant>(name))
            {
                modified = true;
                transient.put(name, datum);
            }
        }
        bool contains(const QString& name)
        {
            return transient.contains(name);
        }
        template<typename T>
        T get(QString& name) {
            return transient.get<T>(name);
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

        bool modifiedp() const { return modified; }
    };

    typedef std::shared_ptr<impl_bundle> pbundle;

    class base_value : public QObject {
        Q_OBJECT
    public:
        virtual QVariant operator=(const QVariant& datum) = 0;
    public slots:
#define DEFINE_SLOT(t) void setValue(t datum) { this->operator=(datum); }
        DEFINE_SLOT(double)
        DEFINE_SLOT(int)
        DEFINE_SLOT(QString)
        DEFINE_SLOT(bool)
    signals:
#define DEFINE_SIGNAL(t) void valueChanged(t);
        DEFINE_SIGNAL(double)
        DEFINE_SIGNAL(int)
        DEFINE_SIGNAL(QString)
        DEFINE_SIGNAL(bool)
    };

    template<typename T>
    class value : public base_value {
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
        QVariant operator=(const QVariant& datum) {
            b->store(self_name, datum);
            switch (datum.type())
            {
#define BRANCH_ON(e, m) case QVariant::e: return valueChanged(datum.m()), datum
            BRANCH_ON(Int, toInt);
            BRANCH_ON(Double, toDouble);
            BRANCH_ON(String, toString);
            BRANCH_ON(Bool, toBool);
            default: abort();
            }
        }
    };

    template<typename T, typename Q>
    inline void tie_setting(value<T>&, Q*);

    template<>
    inline void tie_setting(value<int>& v, QComboBox* cb)
    {
        base_value::connect(cb, SIGNAL(currentIndexChanged(int)), &v, SLOT(setValue(int)));
        base_value::connect(&v, SIGNAL(valueChanged(int)), cb, SLOT(setCurrentIndex(int)));
        cb->setCurrentIndex(v);
    }

    template<>
    inline void tie_setting(value<bool>& v, QCheckBox* cb)
    {
        base_value::connect(cb, SIGNAL(toggled(bool)), &v, SLOT(setValue(bool)));
        base_value::connect(&v, SIGNAL(valueChanged(bool)), cb, SLOT(setChecked(bool)));
        cb->setChecked(v);
    }

    template<>
    inline void tie_setting(value<double>& v, QDoubleSpinBox* dsb)
    {
        base_value::connect(dsb, SIGNAL(valueChanged(double)), &v, SLOT(setValue(double)));
        base_value::connect(&v, SIGNAL(valueChanged(double)), dsb, SLOT(setValue(double)));
        dsb->setValue(v);
    }

    template<>
    inline void tie_setting(value<int>& v, QSpinBox* sb)
    {
        base_value::connect(sb, SIGNAL(valueChanged(int)), &v, SLOT(setValue(int)));
        base_value::connect(&v, SIGNAL(valueChanged(int)), sb, SLOT(setValue(int)));
        sb->setValue(v);
    }

    template<>
    inline void tie_setting(value<int>& v, QSlider* sl)
    {
        base_value::connect(sl, SIGNAL(valueChanged(int)), &v, SLOT(setValue(int)));
        base_value::connect(&v, SIGNAL(valueChanged(int)), sl, SLOT(setValue(int)));
        sl->setValue(v);
    }

    inline pbundle bundle(const QString& group) {
        return std::make_shared<impl_bundle>(group);
    }
}
