/* Copyright (c) 2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "compat/qhash.hpp"

#include <unordered_map>
#include <vector>

#include <QString>
#include <QRecursiveMutex>

#include "export.hpp"

namespace options {
    class value_;
}

namespace options::detail {

class OTR_OPTIONS_EXPORT connector
{
    friend class ::options::value_;

    using value_type = value_*;
    using value_vec = std::vector<value_type>;
    std::unordered_map<QString, value_vec> connected_values;

    void on_value_destructed(value_type val);
    void on_value_created(value_type val);

protected:
    void notify_values(const QString& name) const;
    void notify_all_values() const;
    virtual QRecursiveMutex* get_mtx() const = 0;
    void set_all_to_default_();

public:
    connector();
    virtual ~connector();
};

} // ns options::detail
