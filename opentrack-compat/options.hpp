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
#include <cinttypes>
#include <vector>
#include <memory>

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
#include <QApplication>
#include <QMetaType>

#include <QDebug>

#ifdef BUILD_compat
#   include "compat-export.hpp"
#else
#   include "compat-import.hpp"
#endif

template<typename t> using mem = std::shared_ptr<t>;

#define OPENTRACK_CONFIG_FILENAME_KEY "settings-filename"
#define OPENTRACK_DEFAULT_CONFIG "default.ini"
#define OPENTRACK_ORG "opentrack-2.3"

namespace options
{
    struct OPENTRACK_COMPAT_EXPORT slider_value
    {
        double cur, min, max;
        slider_value(double cur, double min, double max) :
            cur(cur),
            min(min),
            max(max)
        {}
        slider_value() : slider_value(0, 0, 0) {}
        operator double() const;
        double to_abs() const { return operator double(); }
        static slider_value from_abs(double val, double min, double max);
    };
}

Q_DECLARE_METATYPE(options::slider_value)

namespace options {
    template<typename k, typename v> using map = std::map<k, v>;

    // snapshot of qsettings group at given time
    class OPENTRACK_COMPAT_EXPORT group {
    private:
        map<QString, QVariant> kvs;
        QString name;
    public:
        group(const QString& name);
        void save() const;
        void put(const QString& s, const QVariant& d);
        bool contains(const QString& s) const;
        static QString ini_directory();
        static QString ini_filename();
        static QString ini_pathname();
        static const QStringList ini_list();
        static const mem<QSettings> ini_file();

        template<typename t>
        t get(const QString& k) const
        {
            auto value = kvs.find(k);
            if (value != kvs.cend())
                return value->second.value<t>();
            return t();
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
        void saving() const;
    public:
        impl_bundle(const QString& group_name);
        QString name() { return group_name; }
        void reload();
        void store_kv(const QString& name, const QVariant& datum);
        bool contains(const QString& name) const;
        void save();
        bool modifiedp() const;

        template<typename t>
        t get(const QString& name) const
        {
            QMutexLocker l(const_cast<QMutex*>(&mtx));
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

    pbundle bundle(const QString& name);

    class OPENTRACK_COMPAT_EXPORT opt_bundle final : public impl_bundle
    {
    public:
        opt_bundle() : impl_bundle("i-have-no-name") {}
        opt_bundle(const QString& group_name);
        ~opt_bundle();
    };

#define DEFINE_SLOT(t) void setValue(t datum) { store(datum); }
#define DEFINE_SIGNAL(t) void valueChanged(t)

    class OPENTRACK_COMPAT_EXPORT base_value : public QObject
    {
        Q_OBJECT
    public:
        QString name() const { return self_name; }
        base_value(pbundle b, const QString& name);
    signals:
        DEFINE_SIGNAL(double);
        DEFINE_SIGNAL(int);
        DEFINE_SIGNAL(bool);
        DEFINE_SIGNAL(QString);
        DEFINE_SIGNAL(slider_value);
    protected:
        pbundle b;
        QString self_name;

        template<typename t>
        void store(const t& datum)
        {
            b->store_kv(self_name, QVariant::fromValue(datum));
            emit valueChanged(static_cast<t>(datum));
        }
    public slots:
        DEFINE_SLOT(double)
        DEFINE_SLOT(int)
        DEFINE_SLOT(QString)
        DEFINE_SLOT(bool)
        DEFINE_SLOT(slider_value)
    public slots:
        virtual void reload() = 0;
    };

    template<typename t_>
    class value : public base_value
    {
        template<typename t__, typename Enable = void>
        struct get_t
        { using t = t__; };

        // Qt uses int a lot in slots so use it for all enums
        template<typename t__>
        struct get_t<t__, typename std::enable_if<std::is_enum<t__>::value>::type>
        //{ using t = typename std::underlying_type<t__>::type; };
        { using t = int; };

        using t = t_;
    public:
        using underlying_t = typename get_t<t_>::t;

        t operator=(const t datum)
        {
            store(static_cast<underlying_t>(datum));
            return datum;
        }

        static constexpr const Qt::ConnectionType DIRECT_CONNTYPE = Qt::AutoConnection;
        static constexpr const Qt::ConnectionType SAFE_CONNTYPE = Qt::QueuedConnection;

        value(pbundle b, const QString& name, t def) : base_value(b, name), def(static_cast<underlying_t>(def))
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
            return static_cast<t>(b->contains(self_name) ? b->get<underlying_t>(self_name) : def);
        }

        void reload() override
        {
            *this = static_cast<t>(*this);
        }
    private:
        underlying_t def;
    };

    struct OPENTRACK_COMPAT_EXPORT opts
    {
        pbundle b;
        opts(const QString& name);
        opts& operator=(const opts&) = delete;
        opts(const opts&) = delete;
        ~opts();
    };

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
        enum_cases.reserve(cb->count());

        for (int i = 0; i < cb->count(); i++)
            enum_cases.push_back(cb->itemData(i).toInt());

        struct fn1
        {
            value<t>& v;
            QComboBox* cb;
            std::vector<int> enum_cases;

            fn1(value<t>& v, QComboBox* cb, const std::vector<int>& enum_cases) : v(v), cb(cb), enum_cases(enum_cases)
            {}

            void operator()(int idx)
            {
                if (idx < 0 || idx >= (int)enum_cases.size())
                    v = static_cast<t>(-1);
                else
                    v = static_cast<t>(enum_cases[idx]);
            }
        };

        struct fn2
        {
            value<t>& v;
            QComboBox* cb;
            std::vector<int> enum_cases;

            fn2(value<t>& v, QComboBox* cb, const std::vector<int>& enum_cases) : v(v), cb(cb), enum_cases(enum_cases)
            {}

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
                            static_cast<void (base_value::*)(int)>(&base_value::valueChanged),
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
        base_value::connect(&v, SIGNAL(valueChanged(int)), cb, SLOT(setCurrentIndex(int)), v.DIRECT_CONNTYPE);
    }

    template<>
    inline void tie_setting(value<QString>& v, QComboBox* cb)
    {
        cb->setCurrentText(v);
        v = cb->currentText();
        base_value::connect(cb, SIGNAL(currentTextChanged(QString)), &v, SLOT(setValue(QString)), v.DIRECT_CONNTYPE);
        base_value::connect(&v, SIGNAL(valueChanged(QString)), cb, SLOT(setCurrentText(QString)), v.DIRECT_CONNTYPE);
    }

    template<>
    inline void tie_setting(value<bool>& v, QCheckBox* cb)
    {
        cb->setChecked(v);
        base_value::connect(cb, SIGNAL(toggled(bool)), &v, SLOT(setValue(bool)), v.DIRECT_CONNTYPE);
        base_value::connect(&v, SIGNAL(valueChanged(bool)), cb, SLOT(setChecked(bool)), v.DIRECT_CONNTYPE);
    }

    template<>
    inline void tie_setting(value<double>& v, QDoubleSpinBox* dsb)
    {
        dsb->setValue(v);
        base_value::connect(dsb, SIGNAL(valueChanged(double)), &v, SLOT(setValue(double)), v.DIRECT_CONNTYPE);
        base_value::connect(&v, SIGNAL(valueChanged(double)), dsb, SLOT(setValue(double)), v.DIRECT_CONNTYPE);
    }

    template<>
    inline void tie_setting(value<int>& v, QSpinBox* sb)
    {
        sb->setValue(v);
        base_value::connect(sb, SIGNAL(valueChanged(int)), &v, SLOT(setValue(int)), v.DIRECT_CONNTYPE);
        base_value::connect(&v, SIGNAL(valueChanged(int)), sb, SLOT(setValue(int)), v.DIRECT_CONNTYPE);
    }

    template<>
    inline void tie_setting(value<int>& v, QSlider* sl)
    {
        sl->setValue(v);
        v = sl->value();
        base_value::connect(sl, SIGNAL(valueChanged(int)), &v, SLOT(setValue(int)), v.DIRECT_CONNTYPE);
        base_value::connect(&v, SIGNAL(valueChanged(int)), sl, SLOT(setValue(int)), v.DIRECT_CONNTYPE);
    }

    template<>
    inline void tie_setting(value<QString>& v, QLineEdit* le)
    {
        le->setText(v);
        base_value::connect(le, SIGNAL(textChanged(QString)), &v, SLOT(setValue(QString)), v.DIRECT_CONNTYPE);
        base_value::connect(&v, SIGNAL(valueChanged(QString)),le, SLOT(setText(QString)), v.DIRECT_CONNTYPE);
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
        base_value::connect(&v, SIGNAL(valueChanged(int)), t, SLOT(setCurrentIndex(int)), v.DIRECT_CONNTYPE);
    }

    template<>
    inline void tie_setting(value<slider_value>& v, QSlider* w)
    {
        // we can't get these at runtime since signals cross threads
        const int min = w->minimum();
        const int max = w->maximum();
        const int max_ = max - min;

        slider_value sv(v);

        w->setValue(int(sv.cur * max_) + min);
        v = slider_value(max_ <= 0 ? 0 : (w->value() - min) / (double)max_, sv.min, sv.max);

        base_value::connect(w, &QSlider::valueChanged, &v,
                         [=, &v](int pos) -> void
        {
            v = slider_value(max_ <= 0 ? 0 : (pos - min) / (double)max_, sv.min, sv.max);
        },
        v.DIRECT_CONNTYPE);
        base_value::connect(&v, static_cast<void(base_value::*)(double)>(&base_value::valueChanged), w,
                            [=](double value) -> void
        {
            w->setValue(int(value * max_) + min);
        },
        v.DIRECT_CONNTYPE);
    }
}

