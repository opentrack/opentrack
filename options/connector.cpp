/* Copyright (c) 2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "compat/util.hpp"
#include "connector.hpp"
#include "value.hpp"

#include <utility>

namespace options {
namespace detail {

static bool generic_is_equal(const QVariant& val1, const QVariant& val2)
{
    return val1 == val2;
}

connector::~connector() {}

bool connector::is_equal(const QString& name, const QVariant& val1, const QVariant& val2) const
{
    QMutexLocker l(get_mtx());

    auto it = connected_values.find(name);

    if (it != connected_values.cend() && std::get<0>((*it).second).size() != 0)
        return std::get<1>((*it).second)(val1, val2);
    else
        return generic_is_equal(val1, val2);
}

bool connector::on_value_destructed_impl(const QString& name, const base_value* val)
{
    QMutexLocker l(get_mtx());

    const bool ok = progn(
        auto it = connected_values.find(name);

        if (it != connected_values.end())
        {
            std::vector<const base_value*>& values = std::get<0>((*it).second);
            for (auto it = values.begin(); it != values.end(); it++)
            {
                if (*it == val)
                {
                    values.erase(it);
                    return true;
                }
            }
        }
        return false;
    );

    return ok;
}



void connector::on_value_destructed(const QString& name, const base_value* val)
{
    if (!name.size())
        return;

    const bool ok = on_value_destructed_impl(name, val);

    if (!ok)
        qWarning() << "options/connector: value destructed without creating;"
                   << "bundle"
                   << val->b->name()
                   << "value-name" << name
                   << "value-ptr" << quintptr(val);
}

void connector::on_value_created(const QString& name, const base_value* val)
{
    if (!name.size())
        return;

    QMutexLocker l(get_mtx());

    if (on_value_destructed_impl(name, val))
    {
        qWarning() << "options/connector: value created twice;"
                   << "bundle"
                   << val->b->name()
                   << "value-name" << name
                   << "value-ptr" << quintptr(val);
    }

    auto it = connected_values.find(name);

    if (it != connected_values.end())
    {
        tt& tmp = (*it).second;
        std::type_index& typeidx = std::get<2>(tmp);
        std::vector<const base_value*>& values = std::get<0>(tmp);

        if (typeidx != val->type_index)
            std::get<1>((*it).second) = generic_is_equal;
        values.push_back(val);
    }
    else
    {
        std::vector<const base_value*> vec;
        vec.push_back(val);
        connected_values.emplace(name, tt(vec, val->cmp, val->type_index));
    }
}

void connector::notify_values(const QString& name) const
{
    auto it = connected_values.find(name);
    if (it != connected_values.cend())
    {
        for (const base_value* val : std::get<0>((*it).second))
        {
            val->bundle_value_changed();
        }
    }
}

void connector::notify_all_values() const
{
    for (auto& pair : connected_values)
        for (const base_value* val : std::get<0>(pair.second))
            val->bundle_value_changed();
}

connector::connector()
{
}

}

}
