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
#include <string>

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
    using std::string;
    
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
        map<string, QVariant> map;
        string name;
        static const QString ini_pathname()
        {
            QSettings settings(group::org);
            return settings.value("SettingsFile",
                                  QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
        }
    public:
        group(const string& name) : name(name)
        {
            QSettings conf(ini_pathname(), QSettings::IniFormat);
            auto q_name = QString::fromStdString(name);
            conf.beginGroup(q_name);
            for (auto& k_ : conf.childKeys())
            {
                auto tmp = k_.toUtf8();
                string k(tmp);
                map[k] = conf.value(k_);
            }
            conf.endGroup();
        }
        static constexpr const char* org = "opentrack";
        
        void save()
        {
            QSettings s(ini_pathname(), QSettings::IniFormat);
            auto q_name = QString::fromStdString(name);
            s.beginGroup(q_name);
            for (auto& i : map)
            {
                auto k = QString::fromStdString(i.first);
                s.setValue(k, i.second);
            }
            s.endGroup();
        }
        
        template<typename t>
        t get(const string& k)
        {
            return qcruft_to_t<t>(map[k]);
        }
        
        void put(const string& s, const QVariant& d)
        {
            map[s] = d;
        }
        
        bool contains(const string& s)
        {
            return map.count(s) != 0;
        }
    };

    class impl_bundle : public QObject {
        Q_OBJECT
    protected:
        QMutex mtx;
        const string group_name;
        group saved;
        group transient;
        bool modified;
        impl_bundle(const impl_bundle&) = delete;
        impl_bundle& operator=(const impl_bundle&) = delete;
    public:
        impl_bundle(const string& group_name) :
            mtx(QMutex::Recursive),
            group_name(group_name),
            saved(group_name),
            transient(saved),
            modified(false)
        {
        }
        
        string name() { return group_name; }
        
        void reload() {
            QMutexLocker l(&mtx);
            saved = group(group_name);
            transient = saved;
            modified = false;
        }
        
        bool store_kv(const string& name, const QVariant& datum)
        {
            QMutexLocker l(&mtx);
            
            auto old = transient.get<QVariant>(name);
            if (!transient.contains(name) || datum != old)
            {
                modified = true;
                transient.put(name, datum);
                return true;
            }
            return false;
        }
        bool contains(const string& name)
        {
            QMutexLocker l(&mtx);
            return transient.contains(name);
        }
        template<typename t>
        t get(const string& name)
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
    
    namespace
    {
        template<typename k, typename v, typename cnt = int>
        struct opt_singleton
        {
        public:
            using pbundle = std::shared_ptr<v>;
            using tt = std::tuple<cnt, pbundle>;
        private:
            QMutex implsgl_mtx;
            map<k, tt> implsgl_data;
        public:
            opt_singleton() : implsgl_mtx(QMutex::Recursive) {}
            
            pbundle bundle(const k& key)
            {
                QMutexLocker l(&implsgl_mtx);
                
                if (implsgl_data.count(key) != 0)
                    return std::get<1>(implsgl_data[key]);
                
                auto shr = std::make_shared<v>(key);
                implsgl_data[key] = tt(cnt(1), shr);
                return shr;
            }
            
            void bundle_decf(const k& key)
            {
                QMutexLocker l(&implsgl_mtx);
                
                if (--std::get<0>(implsgl_data[key]) == 0)
                    implsgl_data.erase(key);
            }
            
            ~opt_singleton() { implsgl_data.clear(); }
        };
        
        using pbundle = std::shared_ptr<opt_bundle>;
        using t_fact = opt_singleton<string, opt_bundle>;
        static t_fact* opt_factory = new t_fact;
    }
 
    static inline t_fact::pbundle bundle(const string name) { return opt_factory->bundle(name); }
    
    class opt_bundle : public impl_bundle
    {
    public:
        opt_bundle() : impl_bundle("i-have-no-name") {}
        opt_bundle(const string& group_name) : impl_bundle(group_name) {}
        
        ~opt_bundle()
        {
            opt_factory->bundle_decf(this->group_name);
        }
    };

    class base_value : public QObject {
        Q_OBJECT
#define DEFINE_SLOT(t) void setValue(t datum) { store(datum); }
#define DEFINE_SIGNAL(t) void valueChanged(const t&)
    public:
        base_value(pbundle b, const string& name) : b(b), self_name(name) {}
    protected:
        pbundle b;
        string self_name;
        
        template<typename t>
        void store(const t& datum)
        {
            if (b->store_kv(self_name, datum))
                emit valueChanged(datum);
        }
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
    
    static inline string string_from_qstring(const QString& datum)
    {
        auto tmp = datum.toUtf8();
        return string(tmp.constData());
    }

    template<typename t>
    class value : public base_value {
    public:
        t operator=(const t datum)
        {
            store(datum);
            return datum;
        }
        static constexpr const Qt::ConnectionType DIRECT_CONNTYPE = Qt::DirectConnection;
        static constexpr const Qt::ConnectionType SAFE_CONNTYPE = Qt::UniqueConnection;
        value(pbundle b, const string& name, t def) : base_value(b, name)
        {
            if (!b->contains(name) || b->get<QVariant>(name).type() == QVariant::Invalid)
                *this = def;
        }
        value(pbundle b, const QString& name, t def) : value(b, string_from_qstring(name), def) {}
        value(pbundle b, const char* name, t def) : value(b, string(name), def) {}

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
