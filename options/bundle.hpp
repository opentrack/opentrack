/* Copyright (c) 2013-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "group.hpp"
#include "connector.hpp"
#include "compat/qhash.hpp"

#include <memory>
#include <tuple>
#include <unordered_map>
#include <memory>
#include <vector>

#include <QObject>
#include <QString>
#include <QVariant>
#include <QRecursiveMutex>

#include <QDebug>

#include "export.hpp"

namespace options::detail {
    class bundle;
    struct bundler;
} // ns options::detail

namespace options {
    using bundle_ = detail::bundle;
    using bundle = std::shared_ptr<bundle_>;
    OTR_OPTIONS_EXPORT std::shared_ptr<detail::bundle> make_bundle(const QString& name);
} // ns options

namespace options::detail {

class OTR_OPTIONS_EXPORT bundle final : public QObject, public connector
{
    Q_OBJECT

    friend struct bundler;

    mutable QRecursiveMutex mtx;
    const QString group_name;
    group saved;
    group transient;

    void reload_no_notify();

signals:
    void reloading();
    void saving() const;
    void changed() const;

public:
    bundle(const bundle&) = delete;
    bundle& operator=(const bundle&) = delete;

    QRecursiveMutex* get_mtx() const override { return &mtx; }
    QString name() const { return group_name; }

    explicit bundle(const QString& group_name);
    ~bundle() override;

    void store_kv(const QString& name, QVariant&& datum);
    void store_kv(const QString& name, const QVariant& datum);
    bool contains(const QString& name) const;

    QVariant get_variant(const QString& name) const;
    void notify();

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

    static void notify();
    static void reload_no_notify();
    static void reload();

private:
    QRecursiveMutex implsgl_mtx;
    std::unordered_map<k, weak> implsgl_data {};

    void notify_();
    void reload_no_notify_();

    void reload_();

    friend OTR_OPTIONS_EXPORT
    std::shared_ptr<v> options::make_bundle(const QString& name);

    std::shared_ptr<v> make_bundle_(const k& key);
    static bundler& singleton();

    bundler();
    ~bundler();
};

void set_value_to_default(value_* val);

} // ns options::detail
