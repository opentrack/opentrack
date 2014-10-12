/* Copyright (c) 2013-2014 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include <memory>
#include <QObject>
#include <QSettings>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QMutex>
#include <QMutexLocker>
#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QSlider>
#include <QLineEdit>
#include <QLabel>
#include <QCoreApplication>

#include <QDebug>

namespace options {
    template<typename t>
    // don't elide usages of the function, qvariant default implicit
    // conversion results in nonsensical runtime behavior -sh
    inline t qcruft_to_t (const QVariant& datum);

    template<>
    inline int qcruft_to_t<int>(const QVariant& t)
    {
        return t.toInt();
    }

    template<>
    inline QString qcruft_to_t<QString>(const QVariant& t)
    {
        return t.toString();
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
        static const QString ini_pathname()
        {
            QSettings settings(group::org);
            return settings.value("SettingsFile",
                                  QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
        }
    public:
        group(const QString& name) : name(name)
        {
            QSettings conf(ini_pathname(), QSettings::IniFormat);
            conf.beginGroup(name);
            for (auto& k : conf.childKeys())
                map[k] = conf.value(k);
            conf.endGroup();
        }
        static constexpr const char* org = "opentrack";
        
        void save() {
            QSettings s(ini_pathname(), QSettings::IniFormat);
            s.beginGroup(name);
            for (auto& k : map.keys())
                s.setValue(k, map[k]);
            s.endGroup();
        }
        template<typename t>
        t get(const QString& k) {
            return qcruft_to_t<t>(map.value(k));
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

    class impl_bundle : public QObject {
        Q_OBJECT
    private:
        QMutex mtx;
        const QString group_name;
        group saved;
        group transient;
        impl_bundle(const impl_bundle&) = delete;
        impl_bundle& operator=(const impl_bundle&) = delete;
        bool modified;
    signals:
        void changed();
    public:
        impl_bundle(const QString& group_name) :
            mtx(QMutex::Recursive),
            group_name(group_name),
            saved(group_name),
            transient(saved),
            modified(false)
        {
        }
        void reload() {
            QMutexLocker l(&mtx);
            saved = group(group_name);
            transient = saved;
        }
        void store_kv(const QString& name, const QVariant& datum)
        {
            QMutexLocker l(&mtx);
            auto old = transient.get<QVariant>(name);
            if (!transient.contains(name) || datum != old)
            {
                if (!modified)
                    qDebug() << "bundle" << group_name <<
                                "modified due to" << name <<
                                transient.get<QVariant>(name) <<
                                old << "->" << datum;
                modified = true;
                transient.put(name, datum);
            }
        }
        bool contains(const QString& name)
        {
            QMutexLocker l(&mtx);
            return transient.contains(name);
        }
        template<typename t>
        t get(const QString& name) {
            QMutexLocker l(&mtx);
            return transient.get<t>(name);
        }
        void save()
        {
            QMutexLocker l(&mtx);
            modified = false;
            saved = transient;
            transient.save();
            emit changed();
        }
        void revert()
        {
            QMutexLocker l(&mtx);
            modified = false;
            transient = saved;
            emit changed();
        }

        bool modifiedp() {
            QMutexLocker l(&mtx);
            return modified;
        }
    };

    using pbundle = std::shared_ptr<impl_bundle>;

    class base_value : public QObject {
        Q_OBJECT
#define DEFINE_SLOT(t) void setValue(t datum) { store(datum); }
#define DEFINE_SIGNAL(t) void valueChanged(t);
    public:
        base_value(pbundle b, const QString& name) : b(b), self_name(name) {}
    public slots:
        DEFINE_SLOT(double)
        DEFINE_SLOT(int)
        DEFINE_SLOT(QString)
        DEFINE_SLOT(bool)
    signals:
        DEFINE_SIGNAL(double);
        DEFINE_SIGNAL(int);
        DEFINE_SIGNAL(bool);
        DEFINE_SIGNAL(QString);
        // Qt5 moc really insists on that one -sh 20141012
        DEFINE_SIGNAL(QVariant);
    protected:
        pbundle b;
        QString self_name;
        
        template<typename t>
        void store(const t& datum)
        {
            b->store_kv(self_name, datum);
            emit valueChanged(static_cast<t>(datum));
        }
    };

    template<typename t>
    class value : public base_value {
    public:
        t operator=(const t& datum)
        {
            store(qVariantFromValue<t>(datum));
            return datum;
        }
        static constexpr const Qt::ConnectionType DIRECT_CONNTYPE = Qt::DirectConnection;
        static constexpr const Qt::ConnectionType SAFE_CONNTYPE = Qt::AutoConnection;
        value(pbundle b, const QString& name, t def) : base_value(b, name)
        {
            if (!b->contains(name) || b->get<QVariant>(name).type() == QVariant::Invalid)
                *this = def;
        }
        operator t()
        {
            return b->get<t>(self_name);
        }
    };

    template<typename t, typename q>
    inline void tie_setting(value<t>&, q*);

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
        base_value::connect(&v, SIGNAL(valueChanged(QString)), lb, SLOT(setText(QString)), v.SAFE_CONNTYPE);
    }

    inline pbundle bundle(const QString& group) {
        return std::make_shared<impl_bundle>(group);
    }
}
