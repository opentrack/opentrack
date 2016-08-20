#include "compat/util.hpp"
#include "connector.hpp"
#include "value.hpp"

namespace options {
namespace detail {

connector::~connector() {}

bool connector::on_value_destructed_impl(const QString& name, const base_value* val)
{
    QMutexLocker l(get_mtx());

    const bool ok = progn(
        auto it = connected_values.find(name);

        if (it != connected_values.end())
        {
            std::vector<const base_value*>& values = (*it).second;
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
    const bool ok = on_value_destructed_impl(name, val);

    if (!ok)
        qWarning() << "options/connector: value destructed without creating;"
                   << "bundle"
                   << (val && val->b ? val->b->name() : "<NULL>")
                   << "value-name" << name
                   << "value-ptr" << quintptr(val);
}

void connector::on_value_created(const QString& name, const base_value* val)
{
    QMutexLocker l(get_mtx());

    if (on_value_destructed_impl(name, val))
    {
        qWarning() << "options/connector: value created twice;"
                   << "bundle"
                   << (val && val->b ? val->b->name() : "<NULL>")
                   << "value-name" << name
                   << "value-ptr" << quintptr(val);
    }

    auto it = connected_values.find(name);

    if (it != connected_values.end())
    {
        std::vector<const base_value*>& values = (*it).second;
        values.push_back(val);
    }
    else
    {
        std::vector<const base_value*> vec;
        vec.push_back(val);
        connected_values[name] = vec;
    }
}

void connector::notify_values(const QString& name) const
{
    auto it = connected_values.find(name);
    if (it != connected_values.cend())
    {
        for (const base_value* val : (*it).second)
        {
            val->bundle_value_changed();
        }
    }
}

void connector::notify_all_values() const
{
    for (auto& pair : connected_values)
        for (const base_value* val : pair.second)
            val->bundle_value_changed();
}

connector::connector()
{
}

}

}
