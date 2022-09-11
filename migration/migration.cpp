/* Copyright (c) 2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include <cstdlib>

#include "migration.hpp"

#include "options/options.hpp"

#include <QString>
#include <QSettings>
#include <QChar>

#include <QDebug>

#include <memory>

using namespace options::globals;

// individual migrations are run in the UI thread. they can be interactive if necessary.

namespace migrations::detail {


std::vector<mptr>& migrator::migration_list()
{
    static std::vector<mptr> v;
    return v;
}

std::vector<mfun>& migrator::migration_thunks()
{
    static std::vector<mfun> v;
    return v;
}


void migrator::register_migration(mptr const& m)
{
    const QString date = m->unique_date();

    for (mptr const& m2 : migration_list())
        if (m2->unique_date() == date)
            std::abort();

    if (date.size() != 11)
        abort();

    if (date[8] != QChar('_'))
        abort();

    const QString year = date.left(4);
    const QString month = date.mid(4, 2);
    const QString day = date.mid(6, 2);
    const QString serial = date.mid(9, 2);

    bool ok = true;

    const int year_  = to_int(year, ok),
              month_ = to_int(month, ok),
              day_   = to_int(day, ok);

    (void)to_int(serial, ok);

    if (!ok || year_ < 1970)
        abort();

    if (month_ < 1 || month_ > 12)
        abort();

    if (day_ < 1 || day_ > 31)
        abort();

    migration_list().push_back(m);
}

void migrator::eval_thunks()
{
    for (auto& fun : migration_thunks())
    {
        mptr m = fun();
        register_migration(m);
    }
    if (!migration_thunks().empty())
        sort_migrations();
    migration_thunks().clear();
}

void migrator::add_migration_thunk(mfun& thunk)
{
    migration_thunks().push_back(thunk);
}

std::vector<mptr>& migrator::migrations()
{
    eval_thunks();
    return migration_list();
}

void migrator::sort_migrations()
{
    std::sort(migration_list().begin(), migration_list().end(),
              [](const mptr x, const mptr y) {
        return x->unique_date() < y->unique_date();
    });
}

QString migrator::last_migration_time()
{
    QString ret;

    with_settings_object([&](QSettings& s) {
        s.beginGroup("migrations");
        ret = s.value("last-migration-at", "19700101_00").toString();
        s.endGroup();
    });

    return ret;
}

QString migrator::time_after_migrations()
{
    const std::vector<mptr>& list = migrations();

    if (list.empty())
        return QStringLiteral("19700101_00");

    QString ret = list[list.size() - 1]->unique_date();
    ret += QStringLiteral("~");

    return ret;
}

void migrator::set_last_migration_time(const QString& val)
{
    with_settings_object([&](QSettings& s) {
        s.beginGroup("migrations");
        const QString old_value = s.value("last-migration-at", "").toString();
        if (val != old_value)
        {
            s.setValue("last-migration-at", val);
            options::globals::detail::mark_ini_modified();
        }
        s.endGroup();
    });
}

int migrator::to_int(const QString& str, bool& ok)
{
    bool tmp = false;
    const int ret = int(str.toUInt(&tmp));
    ok &= tmp;
    return ret;
}

std::vector<QString> migrator::run()
{
    std::vector<QString> done;

    const QString last_migration = last_migration_time();

    with_settings_object([&](QSettings&) {
        for (mptr const& m : migrations())
        {
            const QString date = m->unique_date();

            if (date <= last_migration)
                continue;

            if (m->should_run())
            {
                const QByteArray name = m->name().toUtf8();
                const QByteArray date = m->unique_date().toUtf8();
                qDebug() << "migrate:" << date.constData() << name.constData();
                m->run();
                done.push_back(m->name());
            }
        }
        mark_profile_as_not_needing_migration();
    });

    return done;
}

} // ns migrations::detail

namespace migrations {

migration::migration() = default;
migration::~migration() = default;

} // ns migrations

std::vector<QString> run_migrations()
{
    return migrations::detail::migrator::run();
}

void mark_profile_as_not_needing_migration()
{
    using m = migrations::detail::migrator;

    m::mark_profile_as_not_needing_migration();
}

void migrations::detail::migrator::mark_profile_as_not_needing_migration()
{
    set_last_migration_time(time_after_migrations());
}
