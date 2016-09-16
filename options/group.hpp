#pragma once

#include "export.hpp"
#include "compat/util.hpp"
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
    QString name;
public:
    std::map<QString, QVariant> kvs;
    group(const QString& name, mem<QSettings> s);
    group(const QString& name);
    void save() const;
    void save_deferred(QSettings& s) const;
    void put(const QString& s, const QVariant& d);
    bool contains(const QString& s) const;
    static QString ini_directory();
    static QString ini_filename();
    static QString ini_pathname();
    static QString ini_combine(const QString& filename);
    static QStringList ini_list();
    static std::shared_ptr<QSettings> ini_file();

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
