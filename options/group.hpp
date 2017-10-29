#pragma once

#include "export.hpp"

#include "compat/util.hpp"

// included here to propagate into callers of options::group
#include "metatype.hpp"

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
    static int ini_refcount;
    static bool ini_modifiedp;
    static QMutex cur_ini_mtx;

    static std::shared_ptr<QSettings> cur_global_ini;
    static int global_ini_refcount;
    static bool global_ini_modifiedp;
    static QMutex global_ini_mtx;

    struct OTR_OPTIONS_EXPORT saver_ final
    {
        QSettings& s;
        int& refcount;
        bool& modifiedp;

        never_inline ~saver_();
        never_inline saver_(QSettings& s, int& refcount, bool& modifiedp);
    };
    static std::shared_ptr<QSettings> cur_settings_object();
    static std::shared_ptr<QSettings> cur_global_settings_object();

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
    static bool is_portable_installation();

    static void mark_ini_modified();

    template<typename t>
    never_inline
    t get(const QString& k) const
    {
        auto value = kvs.find(k);
        if (value != kvs.cend())
            return value->second.value<t>();
        return t();
    }

    template<typename F>
    never_inline
    static auto with_settings_object(F&& fun)
    {
        QMutexLocker l(&cur_ini_mtx);
        saver_ saver { *cur_settings_object(), ini_refcount, ini_modifiedp };

        return fun(saver.s);
    }

    template<typename F>
    static auto with_global_settings_object(F&& fun)
    {
        QMutexLocker l(&global_ini_mtx);
        saver_ saver { *cur_global_settings_object(), global_ini_refcount, global_ini_modifiedp };
        global_ini_modifiedp = true;

        return fun(saver.s);
    }
};

}
