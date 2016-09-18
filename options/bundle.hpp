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

#include "compat/util.hpp"
#include "export.hpp"

namespace options {

namespace detail {

struct bundler;

class OPENTRACK_OPTIONS_EXPORT bundle final : public QObject, public virtual connector
{
    class OPENTRACK_OPTIONS_EXPORT mutex final : public QMutex
    {
    public:
        mutex(QMutex::RecursionMode mode) : QMutex(mode) {}
        operator QMutex*() const { return const_cast<QMutex*>(static_cast<const QMutex*>(this)); }
    };

    Q_OBJECT
private:
    friend bundler;

    mutex mtx;
    const QString group_name;
    group saved;
    group transient;

    bundle(const bundle&) = delete;
    bundle(bundle&&) = delete;
    bundle& operator=(bundle&&) = delete;
    bundle& operator=(const bundle&) = delete;
    QMutex* get_mtx() const override;

signals:
    void reloading();
    void saving() const;
    void changed() const;
public:
    bundle(const QString& group_name);
    ~bundle() override;
    QString name() const { return group_name; }
    void reload(std::shared_ptr<QSettings> settings = group::ini_file());
    void store_kv(const QString& name, const QVariant& datum);
    bool contains(const QString& name) const;
    void save();
    void save_deferred(QSettings& s);
    bool is_modified() const;

    template<typename t>
    t get(const QString& name) const
    {
        QMutexLocker l(mtx);
        return transient.get<t>(name);
    }
};

struct OPENTRACK_OPTIONS_EXPORT bundler
{
public:
    using k = QString;
    using v = bundle;
    using cnt = int;
    using tt = std::tuple<cnt, std::weak_ptr<v>>;
private:
    QMutex implsgl_mtx;
    std::map<k, tt> implsgl_data;
    void after_profile_changed_();
public:
    bundler();
    ~bundler();
    std::shared_ptr<v> make_bundle(const k& key);
    void bundle_decf(const k& key);
    static void refresh_all_bundles();
};

OPENTRACK_OPTIONS_EXPORT bundler& singleton();
}

using bundle_type = detail::bundle;
using bundle = std::shared_ptr<bundle_type>;

OPENTRACK_OPTIONS_EXPORT std::shared_ptr<bundle_type> make_bundle(const QString& name);

}
