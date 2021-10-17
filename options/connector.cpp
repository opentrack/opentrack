/* Copyright (c) 2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "connector.hpp"
#include "value.hpp"

namespace options::detail {

connector::connector() = default;
connector::~connector() = default;

void connector::on_value_destructed(value_type val)
{
    const QString& name = val->name();
    QMutexLocker l(get_mtx());

    const auto it = connected_values.find(name);

    if (it != connected_values.end())
    {
        value_vec& values = it->second;
        for (auto it = values.begin(); it != values.end(); )
        {
            if (*it == val)
                it = values.erase(it);
            else
                it++;
        }
    }

    if (it != connected_values.end() && it->second.empty())
        connected_values.erase(it);
}

void connector::on_value_created(value_type val)
{
    const QString& name = val->name();

    if (name.isEmpty())
        return;

    QMutexLocker l(get_mtx());

    auto it = connected_values.find(name);

    if (it != connected_values.end())
    {
        value_vec& values = it->second;
        values.push_back(val);
    }
    else
    {
        value_vec vec;
        vec.reserve(4);
        vec.push_back(val);
        connected_values.emplace(name, vec);
    }
}

void connector::notify_values(const QString& name) const
{
    QMutexLocker l(get_mtx());

    auto it = connected_values.find(name);
    if (it != connected_values.cend())
        for (value_type val : it->second)
            val->notify();
}

void connector::notify_all_values() const
{
    QMutexLocker l(get_mtx());

    for (const auto& [k, v] : connected_values)
        for (value_type val : v)
            val->notify();
}

void connector::set_all_to_default_()
{
    QMutexLocker l(get_mtx());

    for (auto& pair : connected_values)
        for (auto& val : pair.second)
            val->set_to_default();
}

} // ns options::detail
