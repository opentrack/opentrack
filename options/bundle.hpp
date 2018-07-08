/* Copyright (c) 2013-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "group.hpp"
#include "connector.hpp"

#include <memory>
#include <tuple>
#include <map>
#include <memory>
#include <vector>

#include <QObject>
#include <QString>
#include <QVariant>
#include <QMutex>
#include <QMutexLocker>

#include <QDebug>

#include "export.hpp"

namespace options::detail {
    class bundle;
} // ns options::detail

namespace options {
    using bundle_ = detail::bundle;
    using bundle = std::shared_ptr<bundle_>;
    OTR_OPTIONS_EXPORT std::shared_ptr<detail::bundle> make_bundle(const QString& name);
} // ns options

namespace options::detail {

struct bundler;

class OTR_OPTIONS_EXPORT bundle final : public QObject, public connector
{
    Q_OBJECT

    class OTR_OPTIONS_EXPORT mutex final : public QMutex
    {
    public:
        explicit mutex(QMutex::RecursionMode mode) : QMutex(mode) {}
        operator QMutex*() const { return const_cast<QMutex*>(static_cast<const QMutex*>(this)); }
    };

private:
    mutex mtx;
    const QString group_name;
    group saved;
    group transient;

signals:
    void reloading();
    void saving() const;
    void changed() const;

public:
    bundle(const bundle&) = delete;
    bundle(bundle&&) = delete;
    bundle& operator=(bundle&&) = delete;
    bundle& operator=(const bundle&) = delete;
    QMutex* get_mtx() const override;

    cc_noinline explicit bundle(const QString& group_name);
    cc_noinline ~bundle() override;
    QString name() const { return group_name; }
    cc_noinline void store_kv(const QString& name, const QVariant& datum);
    cc_noinline bool contains(const QString& name) const;
    cc_noinline bool is_modified() const;

    template<typename t>
    t get(const QString& name) const
    {
        QMutexLocker l(mtx);
        return transient.get<t>(name);
    }

public slots:
    void save();
    void reload();
    void set_all_to_default();
};

struct OTR_OPTIONS_EXPORT bundler final
{
    using k = QString;
    using v = bundle;
    using weak = std::weak_ptr<v>;
    using shared = std::shared_ptr<v>;

private:
    QMutex implsgl_mtx { QMutex::Recursive };
    std::map<k, weak> implsgl_data;
    void after_profile_changed_();

public:
    static void refresh_all_bundles();

private:
    friend OTR_OPTIONS_EXPORT
    std::shared_ptr<v> options::make_bundle(const QString& name);

    std::shared_ptr<v> make_bundle_(const k& key);

    static bundler& bundler_singleton();

    bundler();
    ~bundler();
};

void set_base_value_to_default(value_* val);

} // ns options::detail

