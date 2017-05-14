#pragma once

#include "export.hpp"

#include "compat/util.hpp"

#include <map>
#include <memory>
#include <QString>
#include <QList>
#include <QVariant>
#include <QSettings>
#include <QMutex>

namespace options {

// snapshot of qsettings group at given time
class OTR_OPTIONS_EXPORT group final
{
    QString name;

    static QString cur_ini_pathname;
    static std::shared_ptr<QSettings> cur_ini;
    static QMutex cur_ini_mtx;
    static int ini_refcount;
    static bool ini_modifiedp;
    struct OTR_OPTIONS_EXPORT saver_ final
    {
        QSettings& s;
        QMutex& mtx;
        QMutexLocker lck;

        ~saver_();
        saver_(QSettings& s, QMutex&);
    };
    static std::shared_ptr<QSettings> cur_settings_object();

public:
    std::map<QString, QVariant> kvs;
    group(const QString& name);
    void save() const;
    void put(const QString& s, const QVariant& d);
    bool contains(const QString& s) const;
    static QString ini_directory();
    static QString ini_filename();
    static QString ini_pathname();
    static QString ini_combine(const QString& filename);
    static QStringList ini_list();

    static void mark_ini_modified();

    template<typename t>
    OTR_NEVER_INLINE
    t get(const QString& k) const
    {
        auto value = kvs.find(k);
        if (value != kvs.cend())
            return value->second.value<t>();
        return t();
    }

    template<typename F>
    OTR_NEVER_INLINE
    static auto with_settings_object(F&& fun)
    {
        saver_ saver { *cur_settings_object(), cur_ini_mtx };

        return fun(static_cast<QSettings&>(saver.s));
    }
};

}
