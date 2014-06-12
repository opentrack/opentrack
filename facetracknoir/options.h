/* Copyright (c) 2013 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include <QObject>
#include <QSettings>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QMutex>
#include <QMutexLocker>
#include <memory>
#include <cassert>
#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QSlider>
#include <QLineEdit>
#include <QLabel>
#include <QCoreApplication>

#ifdef __GNUC__
#   define ov override
#else
#   define ov
#endif

#include <QDebug>

namespace options {
    template<typename T>
    inline T qcruft_to_t (const QVariant& t);

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
            emit reloaded();
        }

        std::shared_ptr<impl_bundle> make(const QString& name) {
            return std::make_shared<impl_bundle>(name);
        }
        void store(const QString& name, const QVariant& datum)
        {
            QMutexLocker l(&mtx);
            if (!transient.contains(name) || datum != transient.get<QVariant>(name))
            {
                if (!modified)
                    qDebug() << name << transient.get<QVariant>(name) << datum;
                modified = true;
                transient.put(name, datum);
                emit bundleChanged();
            }
        }
        bool contains(const QString& name)
        {
            QMutexLocker l(&mtx);
            return transient.contains(name);
        }
        template<typename T>
        T get(const QString& name) {
            QMutexLocker l(&mtx);
            return transient.get<T>(name);
        }
        void save()
        {
            QMutexLocker l(&mtx);
            modified = false;
            saved = transient;
            transient.save();
        }
        void revert()
        {
            QMutexLocker l(&mtx);
            modified = false;
            transient = saved;
            emit bundleChanged();
        }

        bool modifiedp() {
            QMutexLocker l(&mtx);
            return modified;
        }
    signals:
        void bundleChanged();
        void reloaded();
    };

    typedef std::shared_ptr<impl_bundle> pbundle;

    class base_value : public QObject {
        Q_OBJECT
    public:
        base_value(pbundle b, const QString& name) : b(b), self_name(name) {
            connect(b.get(), SIGNAL(reloaded()), this, SLOT(reread_value()));
        }
    protected:
        virtual QVariant operator=(const QVariant& datum) = 0;
        pbundle b;
        QString self_name;
    public slots:
        void reread_value()
        {
            this->operator=(b->get<QVariant>(self_name));
        }
    public slots:
#define DEFINE_SLOT(t) void setValue(t datum) { this->operator=(qVariantFromValue(datum)); }
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
    protected:
        QVariant operator=(const QVariant& datum) {
            auto foo = qcruft_to_t<T>(datum);
            b->store(self_name, qVariantFromValue<T>(foo));
            emit valueChanged(foo);
            return datum;
        }
    public:
        static constexpr const Qt::ConnectionType QT_CONNTYPE = Qt::UniqueConnection;
        static constexpr const Qt::ConnectionType OPT_CONNTYPE = Qt::UniqueConnection;
        value(pbundle b, const QString& name, T def) :
            base_value(b, name)
        {
            if (!b->contains(name) || b->get<QVariant>(name).type() == QVariant::Invalid)
            {
                this->operator=(qVariantFromValue<T>(def));
            }
        }
        operator T() { return b->get<T>(self_name); }
        QVariant operator=(const T& datum)
        {
            return this->operator =(qVariantFromValue<T>(datum));
        }
    };

    template<typename T, typename Q>
    inline void tie_setting(value<T>&, Q*);

    template<>
    inline void tie_setting(value<int>& v, QComboBox* cb)
    {
        cb->setCurrentIndex(v);
        base_value::connect(cb, SIGNAL(currentIndexChanged(int)), &v, SLOT(setValue(int)), v.QT_CONNTYPE);
        base_value::connect(&v, SIGNAL(valueChanged(int)), cb, SLOT(setCurrentIndex(int)), v.OPT_CONNTYPE);
    }

    template<>
    inline void tie_setting(value<QString>& v, QComboBox* cb)
    {
        cb->setCurrentText(v);
        v = cb->currentText();
        base_value::connect(cb, SIGNAL(currentTextChanged(QString)), &v, SLOT(setValue(QString)), v.QT_CONNTYPE);
        base_value::connect(&v, SIGNAL(valueChanged(QString)), cb, SLOT(setCurrentText(QString)), v.OPT_CONNTYPE);
    }

    template<>
    inline void tie_setting(value<bool>& v, QCheckBox* cb)
    {
        cb->setChecked(v);
        base_value::connect(cb, SIGNAL(toggled(bool)), &v, SLOT(setValue(bool)), v.QT_CONNTYPE);
        base_value::connect(&v, SIGNAL(valueChanged(bool)), cb, SLOT(setChecked(bool)), v.OPT_CONNTYPE);
    }

    template<>
    inline void tie_setting(value<double>& v, QDoubleSpinBox* dsb)
    {
        dsb->setValue(v);
        base_value::connect(dsb, SIGNAL(valueChanged(double)), &v, SLOT(setValue(double)), v.QT_CONNTYPE);
        base_value::connect(&v, SIGNAL(valueChanged(double)), dsb, SLOT(setValue(double)), v.OPT_CONNTYPE);
    }

    template<>
    inline void tie_setting(value<int>& v, QSpinBox* sb)
    {
        sb->setValue(v);
        base_value::connect(sb, SIGNAL(valueChanged(int)), &v, SLOT(setValue(int)), v.QT_CONNTYPE);
        base_value::connect(&v, SIGNAL(valueChanged(int)), sb, SLOT(setValue(int)), v.OPT_CONNTYPE);
    }

    template<>
    inline void tie_setting(value<int>& v, QSlider* sl)
    {
        sl->setValue(v);
        base_value::connect(sl, SIGNAL(valueChanged(int)), &v, SLOT(setValue(int)), v.QT_CONNTYPE);
        base_value::connect(&v, SIGNAL(valueChanged(int)), sl, SLOT(setValue(int)), v.OPT_CONNTYPE);
    }

    template<>
    inline void tie_setting(value<QString>& v, QLineEdit* le)
    {
        le->setText(v);
        base_value::connect(le, SIGNAL(textChanged(QString)), &v, SLOT(setValue(QString)), v.QT_CONNTYPE);
        base_value::connect(&v, SIGNAL(valueChanged(QString)),le, SLOT(setText(QString)), v.OPT_CONNTYPE);
    }

    template<>
    inline void tie_setting(value<QString>& v, QLabel* lb)
    {
        lb->setText(v);
        base_value::connect(&v, SIGNAL(valueChanged(QString)), lb, SLOT(setText(QString)), v.OPT_CONNTYPE);
    }

    inline pbundle bundle(const QString& group) {
        return std::make_shared<impl_bundle>(group);
    }
}
