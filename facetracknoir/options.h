/* Copyright (c) 2013-2014 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include <memory>
#include <tuple>
#include <map>

#include <QObject>
#include <QSettings>
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

#include <cinttypes>

#include <QDebug>

namespace options {
    template<typename k, typename v>
    using map = std::map<k, v>;
    
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
        map<QString, QVariant> map;
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
        
        void save()
        {
            QSettings s(ini_pathname(), QSettings::IniFormat);
            s.beginGroup(name);
            for (auto& i : map)
                s.setValue(i.first, i.second);
            s.endGroup();
        }
        
        template<typename t>
        t get(const QString& k)
        {
            return qcruft_to_t<t>(map[k]);
        }
        
        void put(const QString& s, const QVariant& d)
        {
            map[s] = d;
        }
        
        bool contains(const QString& s)
        {
            return map.count(s) != 0;
        }
    };

    class impl_bundle : public QObject {
        Q_OBJECT
    protected:
        QMutex mtx;
        const QString group_name;
        group saved;
        group transient;
        bool modified;
        impl_bundle(const impl_bundle&) = delete;
        impl_bundle& operator=(const impl_bundle&) = delete;
    public:
        impl_bundle(const QString& group_name) :
            mtx(QMutex::Recursive),
            group_name(group_name),
            saved(group_name),
            transient(saved),
            modified(false)
        {
        }
        
        QString name() { return group_name; }
        
        void reload() {
            QMutexLocker l(&mtx);
            saved = group(group_name);
            transient = saved;
            modified = false;
        }
        
        bool store_kv(const QString& name, const QVariant& datum)
        {
            QMutexLocker l(&mtx);
            
            auto old = transient.get<QVariant>(name);
            if (!transient.contains(name) || datum != old)
            {
                if (!modified)
                    qDebug() << "bundle" << (intptr_t)static_cast<void*>(this) <<
                                "modified as per" << name << old << "->" << datum;
                
                modified = true;
                transient.put(name, datum);
                return true;
            }
            return false;
        }
        bool contains(const QString& name)
        {
            QMutexLocker l(&mtx);
            return transient.contains(name);
        }
        template<typename t>
        t get(const QString& name)
        {
            QMutexLocker l(&mtx);
            return transient.get<t>(name);
        }
        void save()
        {
            QMutexLocker l(&mtx);
            modified = false;
            saved = transient;
            transient.save();
        }

        bool modifiedp() {
            QMutexLocker l(&mtx);
            return modified;
        }
    };
    
    class opt_bundle;
    using pbundle = std::shared_ptr<opt_bundle>;
    
    namespace {
        using tt = std::tuple<int, pbundle>;
        
        QMutex implsgl_mtx(QMutex::Recursive);
        map<QString, tt> implsgl_bundles;
    }
    
    class opt_bundle : public impl_bundle
    {
    public:
        opt_bundle() : impl_bundle("i-have-no-name") {}
        opt_bundle(const QString& group_name) : impl_bundle(group_name) {}
        
        ~opt_bundle()
        {
            QMutexLocker l(&implsgl_mtx);
            
            if (--std::get<0>(implsgl_bundles[this->group_name]) == 0)
            {
                qDebug() << "bundle" << this->group_name << "not used anymore";
                implsgl_bundles.erase(this->group_name);
            }
        }
    };
    
    inline pbundle bundle(const QString& group) {
        QMutexLocker l(&implsgl_mtx);
        
        if (implsgl_bundles.count(group) != 0)
            return std::get<1>(implsgl_bundles[group]);
        
        auto shr = std::make_shared<opt_bundle>(group);
        implsgl_bundles[group] = tt(1,shr);
        return shr;
    }

    class base_value : public QObject {
        Q_OBJECT
#define DEFINE_SLOT(t) void setValue(t datum) { store(datum); }
#define DEFINE_SIGNAL(t) void valueChanged(const t&)
    public:
        base_value(pbundle b, const QString& name) : b(b), self_name(name), reentrancy_count(0) {}
    protected:
        pbundle b;
        QString self_name;
        
        template<typename t>
        void store(const t& datum)
        {
            reentrancy_count++;
            if (b->store_kv(self_name, datum))
                if (reentrancy_count == 0)
                    emit valueChanged(datum);
            reentrancy_count--;
        }
    private:
        volatile char reentrancy_count;
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
    };

    template<typename t>
    class value : public base_value {
    public:
        t operator=(const t datum)
        {
            store(datum);
            return datum;
        }
        static constexpr const Qt::ConnectionType DIRECT_CONNTYPE = Qt::DirectConnection;
        static constexpr const Qt::ConnectionType SAFE_CONNTYPE = Qt::BlockingQueuedConnection;
        value(pbundle b, const QString& name, t def) : base_value(b, name)
        {
            if (!b->contains(name) || b->get<QVariant>(name).type() == QVariant::Invalid)
            {
                qDebug() << "new option" << *(t*)this;
                *this = def;
            }
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
}
