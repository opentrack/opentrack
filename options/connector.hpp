#pragma once

#include <map>
#include <vector>
#include <QString>
#include <QMutex>
#include <QMutexLocker>

#include "export.hpp"

namespace options {

class base_value;

namespace detail {

class OPENTRACK_OPTIONS_EXPORT connector
{
    friend class ::options::base_value;

    std::map<QString, std::vector<const base_value*>> connected_values;

    void on_value_destructed(const QString& name, const base_value* val);
    void on_value_created(const QString& name, const base_value* val);
    bool on_value_destructed_impl(const QString& name, const base_value* val);

protected:
    void notify_values(const QString& name) const;
    void notify_all_values() const;
    virtual QMutex* get_mtx() const = 0;

public:
    connector();
    virtual ~connector();

    connector(const connector&) = default;
    connector& operator=(const connector&) = default;
    connector(connector&&) = default;
    connector& operator=(connector&&) = default;
};

}
}
