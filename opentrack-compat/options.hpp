/* Copyright (c) 2013-2015 Stanislaw Halik
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
#include <QTabWidget>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>

#include <cinttypes>

#include <QDebug>

#include <memory>

#ifdef BUILD_compat
#   include "compat-export.hpp"
#else
#   include "compat-import.hpp"
#endif

template<typename t> using mem = std::shared_ptr<t>;

#define OPENTRACK_CONFIG_FILENAME_KEY "settings-filename"
#define OPENTRACK_DEFAULT_CONFIG "default.ini"
#define OPENTRACK_ORG "opentrack-2.3"

namespace options {
    template<typename k, typename v> using map = std::map<k, v>;

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
    class OPENTRACK_COMPAT_EXPORT group {
    private:
        map<QString, QVariant> kvs;
        QString name;
    public:
        group(const QString& name);
        void save();
        void put(const QString& s, const QVariant& d);
        bool contains(const QString& s);
        static QString ini_directory();
        static QString ini_filename();
        static QString ini_pathname();
        static const QStringList ini_list();
        static const mem<QSettings> ini_file();

        template<typename t>
        t get(const QString& k)
        {
            return qcruft_to_t<t>(kvs[k]);
        }
    };

    class OPENTRACK_COMPAT_EXPORT impl_bundle : public QObject {
        Q_OBJECT
    protected:
        QMutex mtx;
        const QString group_name;
        group saved;
        group transient;
        bool modified;
        impl_bundle(const impl_bundle&) = delete;
        impl_bundle& operator=(const impl_bundle&) = delete;
    signals:
        void reloading();
        void saving();
    public:
        impl_bundle(const QString& group_name);
        QString name() { return group_name; }
        void reload();
        void store_kv(const QString& name, const QVariant& datum);
        bool contains(const QString& name);
        void save();
        bool modifiedp();
        
        template<typename t>
        t get(const QString& name)
        {
            QMutexLocker l(&mtx);
            return transient.get<t>(name);
        }
    };

    class opt_bundle;

    namespace detail
    {
        struct OPENTRACK_COMPAT_EXPORT opt_singleton
        {
        public:
            using k = QString;
            using v = opt_bundle;
            using cnt = int;
            using pbundle = std::shared_ptr<v>;
            using tt = std::tuple<cnt, std::weak_ptr<v>>;
        private:
            QMutex implsgl_mtx;
            map<k, tt> implsgl_data;
        public:
            opt_singleton();
            pbundle bundle(const k& key);
            void bundle_decf(const k& key);
        };
        
        OPENTRACK_COMPAT_EXPORT opt_singleton& singleton();
    }
    
    using pbundle = std::shared_ptr<opt_bundle>;
    
    static inline pbundle bundle(const QString name) { return detail::singleton().bundle(name); }

    class OPENTRACK_COMPAT_EXPORT opt_bundle : public impl_bundle
    {
    public:
        opt_bundle() : impl_bundle("i-have-no-name") {}
        opt_bundle(const QString& group_name);
        ~opt_bundle();
    };

    class OPENTRACK_COMPAT_EXPORT base_value : public QObject
    {
        Q_OBJECT
#define DEFINE_SLOT(t) void setValue(t datum) { store(datum); }
#define DEFINE_SIGNAL(t) void valueChanged(t)
    public:
        QString name() { return self_name; }
        base_value(pbundle b, const QString& name);
    signals:
        DEFINE_SIGNAL(double);
        DEFINE_SIGNAL(int);
        DEFINE_SIGNAL(bool);
        DEFINE_SIGNAL(QString);
    protected:
        pbundle b;
        QString self_name;

        template<typename t>
        void store(const t& datum)
        {
            b->store_kv(self_name, datum);
            emit valueChanged(static_cast<t>(datum));
        }
    public slots:
        DEFINE_SLOT(double)
        DEFINE_SLOT(int)
        DEFINE_SLOT(QString)
        DEFINE_SLOT(bool)
    public slots:
        virtual void reload() = 0;
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
        static constexpr const Qt::ConnectionType SAFE_CONNTYPE = Qt::UniqueConnection;
        value(pbundle b, const QString& name, t def) : base_value(b, name), def(def)
        {
            QObject::connect(b.get(), SIGNAL(reloading()),
                             this, SLOT(reload()),
                             DIRECT_CONNTYPE);
            if (!b->contains(name) || b->get<QVariant>(name).type() == QVariant::Invalid)
                *this = def;
        }
        value(pbundle b, const char* name, t def) : value(b, QString(name), def) {}

        operator t() const
        {
            return b->contains(self_name) ? b->get<t>(self_name) : def;
        }
        void reload() override {
            *this = static_cast<t>(*this);
        }
    private:
        t def;
    };
    
    struct OPENTRACK_COMPAT_EXPORT opts
    {
        pbundle b;
        opts(const QString& name);
        ~opts();
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
    
    template<>
    inline void tie_setting(value<int>& v, QTabWidget* t)
    {
        t->setCurrentIndex(v);
        base_value::connect(t, SIGNAL(currentChanged(int)), &v, SLOT(setValue(int)), v.DIRECT_CONNTYPE);
        base_value::connect(&v, SIGNAL(valueChanged(int)), t, SLOT(setCurrentIndex(int)), v.SAFE_CONNTYPE);
    }
}
