/* Copyright (c) 2013-2016 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

// XXX TODO this header is too long

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
#include <QDataStream>

#include <QDebug>

#include "export.hpp"

template<typename t> using mem = std::shared_ptr<t>;

#define OPENTRACK_CONFIG_FILENAME_KEY "settings-filename"
#define OPENTRACK_DEFAULT_CONFIG "default.ini"
#define OPENTRACK_ORG "opentrack-2.3"

namespace options
{
    class OPENTRACK_COMPAT_EXPORT slider_value final
    {
        double cur_, min_, max_;
    public:
        slider_value(double cur, double min, double max);
        slider_value(const slider_value& v);
        slider_value();
        slider_value& operator=(const slider_value& v);
        bool operator==(const slider_value& v) const;
        operator double() const { return cur_; }
        double cur() const { return cur_; }
        double min() const { return min_; }
        double max() const { return max_; }
        slider_value update_from_slider(int pos, int q_min, int q_max) const;
        int to_slider_pos(int q_min, int q_max) const;
    };
}

QT_BEGIN_NAMESPACE

inline QDebug operator << (QDebug dbg, const options::slider_value& val)
{
    return dbg << val.cur();
}

inline QDataStream& operator << (QDataStream& out, const options::slider_value& v)
{
    out << v.cur()
        << v.min()
        << v.max();
    return out;
}

inline QDataStream& operator >> (QDataStream& in, options::slider_value& v)
{
    double cur, min, max;
    in >> cur;
    in >> min;
    in >> max;
    v = options::slider_value(cur, min, max);
    return in;
}

QT_END_NAMESPACE

Q_DECLARE_METATYPE(options::slider_value)

namespace options {
    namespace {
        class custom_type_initializer
        {
            custom_type_initializer();
            static custom_type_initializer singleton;
        };
    }

    template<typename k, typename v> using map = std::map<k, v>;

    // snapshot of qsettings group at given time
    class OPENTRACK_COMPAT_EXPORT group
    {
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
        bool operator==(const group& other) const;
        bool operator!=(const group& other) const { return !(*this == other); }

        template<typename t>
        t get(const QString& k) const
        {
            auto value = kvs.find(k);
            if (value != kvs.cend())
                return value->second.value<t>();
            return t();
        }
    };

    class OPENTRACK_COMPAT_EXPORT impl_bundle : public QObject
    {
        Q_OBJECT
    protected:
        QMutex mtx;
        const QString group_name;
        group saved;
        group transient;
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

    namespace detail
    {
        class OPENTRACK_COMPAT_EXPORT opt_bundle final : public impl_bundle
        {
        public:
            opt_bundle(const QString& group_name);
            ~opt_bundle();
        };

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

    using pbundle = std::shared_ptr<detail::opt_bundle>;

    inline pbundle bundle(const QString& name)
    {
         return detail::singleton().bundle(name);
    }

#define OPENTRACK_DEFINE_SLOT(t) void setValue(t datum) { store(datum); }
#define OPENTRACK_DEFINE_SIGNAL(t) void valueChanged(t)

    class OPENTRACK_COMPAT_EXPORT base_value : public QObject
    {
        Q_OBJECT
    public:
        QString name() const { return self_name; }
        base_value(pbundle b, const QString& name);
    signals:
        OPENTRACK_DEFINE_SIGNAL(double);
        OPENTRACK_DEFINE_SIGNAL(float);
        OPENTRACK_DEFINE_SIGNAL(int);
        OPENTRACK_DEFINE_SIGNAL(bool);
        OPENTRACK_DEFINE_SIGNAL(const QString&);
        OPENTRACK_DEFINE_SIGNAL(const slider_value&);
    protected:
        pbundle b;
        QString self_name;

        template<typename t>
        void store(const t& datum)
        {
            b->store_kv(self_name, QVariant::fromValue(datum));
            emit valueChanged(static_cast<t>(datum));
        }
        void store(float datum)
        {
            store(double(datum));
        }

    public slots:
        OPENTRACK_DEFINE_SLOT(double)
        OPENTRACK_DEFINE_SLOT(int)
        OPENTRACK_DEFINE_SLOT(bool)
        OPENTRACK_DEFINE_SLOT(const QString&)
        OPENTRACK_DEFINE_SLOT(const slider_value&)
    public slots:
        virtual void reload() = 0;
    };

    namespace detail {
        template<typename t>
        struct value_get_traits
        {
            static inline t get(const t& val, const t&)
            {
                return val;
            }
        };

        template<>
        struct value_get_traits<slider_value>
        {
            using t = slider_value;
            static inline t get(const t& val, const t& def)
            {
                return t(val.cur(), def.min(), def.max());
            }
        };
    }

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

        value(pbundle b, const QString& name, t def) : base_value(b, name), def(def)
        {
            QObject::connect(b.get(), SIGNAL(reloading()),
                             this, SLOT(reload()),
                             DIRECT_CONNTYPE);
            if (!b->contains(name) || b->get<QVariant>(name).type() == QVariant::Invalid)
                *this = def;
        }

        value(pbundle b, const char* name, t def) : value(b, QString(name), def)
        {
        }

        operator t() const
        {
            t val = b->contains(self_name)
                    ? static_cast<t>(b->get<underlying_t>(self_name))
                    : def;
            return detail::value_get_traits<t>::get(val, def);
        }

        void reload() override
        {
            *this = static_cast<t>(*this);
        }
    private:
        t def;
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
        const int q_min = w->minimum();
        const int q_max = w->maximum();
        const int q_diff = q_max - q_min;

        slider_value sv(v);

        const double sv_max = sv.max();
        const double sv_min = sv.min();
        const double sv_c = sv_max - sv_min;

        w->setValue(int((sv.cur() - sv_min) / sv_c * q_diff + q_min));
        v = slider_value(q_diff <= 0 ? 0 : (w->value() - q_min) * sv_c / (double)q_diff + sv_min, sv_min, sv_max);

        base_value::connect(w,
                            &QSlider::valueChanged,
                            &v,
                            [=, &v](int pos) -> void
        {
            if (q_diff <= 0 || pos <= 0)
                v = slider_value(sv_min, sv_min, sv_max);
            else
                v = slider_value((pos - q_min) * sv_c / (double)q_diff + sv_min, sv_min, sv_max);
        },
        v.DIRECT_CONNTYPE);
        base_value::connect(&v,
                            static_cast<void(base_value::*)(double)>(&base_value::valueChanged),
                            w,
                            [=](double value) -> void
        {
            w->setValue(int(value * q_diff) + q_min);
        },
        v.DIRECT_CONNTYPE);
    }
}

