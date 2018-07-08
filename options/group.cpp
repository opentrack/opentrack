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
#include <QDebug>

namespace options::detail {

using namespace options::globals;

group::group(const QString& name) : name(name)
{
    if (name == "")
        return;

    with_settings_object([&](QSettings& conf) {
        conf.beginGroup(name);
        for (auto& k_ : conf.childKeys())
        {
            auto tmp = k_.toUtf8();
            QString k(tmp);
            QVariant val = conf.value(k_);
            if (val.type() != QVariant::Invalid)
                kvs[k] = std::move(val);
        }
        conf.endGroup();
    });
}

void group::save() const
{
    if (name == "")
        return;

    with_settings_object([&](QSettings& s) {
        s.beginGroup(name);
        for (auto& i : kvs)
            s.setValue(i.first, i.second);
        s.endGroup();

        mark_ini_modified();
    });
}

void group::put(const QString &s, const QVariant &d)
{
    kvs[s] = d;
}

bool group::contains(const QString &s) const
{
    const auto it = kvs.find(s);
    return it != kvs.cend() && it->second != QVariant::Invalid;
}

QVariant group::get_variant(const QString& name) const
{
    auto it = kvs.find(name);
    if (it != kvs.cend())
        return it->second;

    return {};
}

} // ns options::detail

