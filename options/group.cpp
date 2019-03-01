/* Copyright (c) 2013-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "group.hpp"
#include "defs.hpp"
#include "globals.hpp"

#include <utility>
#include <algorithm>

#include <QFile>
#include <QDir>

namespace options::detail {

using namespace options::globals;

group::group(const QString& name) : name(name)
{
    constexpr unsigned reserved_buckets = 64;
    kvs.reserve(reserved_buckets);
    kvs.max_load_factor(0.4375);

    if (name.isEmpty())
        return;

    with_settings_object([&](QSettings& conf) {
        conf.beginGroup(name);
        for (auto const& k : conf.childKeys())
            kvs[k] = conf.value(k);
        conf.endGroup();
    });
}

void group::save() const
{
    if (name.isEmpty())
        return;

    with_settings_object([&](QSettings& s) {
        s.beginGroup(name);
        for (auto const& i : kvs)
            s.setValue(i.first, i.second);
        s.endGroup();
    });
}

void group::put(const QString& s, QVariant&& d)
{
    if (d.isNull())
        kvs.erase(s);
    else
        kvs[s] = d;
}

void group::put(const QString& s, const QVariant& d)
{
    put(s, QVariant{d});
}

bool group::contains(const QString& s) const
{
    const auto it = kvs.find(s);
    return it != kvs.cend();
}

QVariant group::get_variant(const QString& name) const
{
    auto it = kvs.find(name);
    if (it != kvs.cend())
        return it->second;

    return {};
}

} // ns options::detail

