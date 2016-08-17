#pragma once

#include "export.hpp"
#include <map>
#include <memory>
#include <QString>
#include <QList>
#include <QVariant>
#include <QSettings>

namespace options {

// snapshot of qsettings group at given time
class OPENTRACK_OPTIONS_EXPORT group final
{
private:
    std::map<QString, QVariant> kvs;
    QString name;
public:
    group(const QString& name);
    void save() const;
    void put(const QString& s, const QVariant& d);
    bool contains(const QString& s) const;
    static QString ini_directory();
    static QString ini_filename();
    static QString ini_pathname();
    static const QStringList ini_list();
    static const std::shared_ptr<QSettings> ini_file();
    bool operator==(const group& other) const;
    bool operator!=(const group& other) const { return !(*this == other); }

    template<typename t>
    t get(const QString& k) const
    {
        auto value = kvs.find(k);
        if (value != kvs.cend())
            return value->second.value<t>();
        return t();
    }
};

}
