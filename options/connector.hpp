/* Copyright (c) 2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <map>
#include <vector>
#include <tuple>
#include <typeinfo>
#include <typeindex>
#include <QVariant>
#include <QString>
#include <QMutex>
#include <QMutexLocker>

#include "export.hpp"

namespace options {

class base_value;

namespace detail {

class OTR_OPTIONS_EXPORT connector
{
    friend class ::options::base_value;

    using value_type = base_value*;
    using value_vec = std::vector<value_type>;
    using comparator = bool(*)(const QVariant&, const QVariant&);
    using tt = std::tuple<value_vec, comparator, std::type_index>;
    std::map<QString, tt> connected_values;

    void on_value_destructed(const QString& name, value_type val);
    void on_value_created(const QString& name, value_type val);
    bool on_value_destructed_impl(const QString& name, value_type val);

protected:
    void notify_values(const QString& name) const;
    void notify_all_values() const;
    virtual QMutex* get_mtx() const = 0;

    template<typename F>
    void forall(F&& fun)
    {
        QMutexLocker l(get_mtx());

        for (auto& pair : connected_values)
            for (auto& val : std::get<0>(pair.second))
                fun(pair.first, val);
    }

public:
    connector();
    virtual ~connector();

    bool is_equal(const QString& name, const QVariant& val1, const QVariant& val2) const;

    connector(const connector&) = default;
    connector& operator=(const connector&) = default;
    connector(connector&&) = default;
    connector& operator=(connector&&) = default;
};

} // ns detail
} // ns options
