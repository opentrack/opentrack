/* Copyright (c) 2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include <cstdlib>

#include "migration.hpp"

#include "options/options.hpp"
#include "compat/util.hpp"

#include <QString>
#include <QSettings>
#include <QChar>

#include <QDebug>

#include <memory>

// individual migrations are run in the UI thread. they can be interactive if necessary.

namespace migrations {

namespace detail {

void migrator::register_migration(migration* m)
{
    const QString date = m->unique_date();

    for (migration* m2 : migrations())
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

    if (year < "2016")
        abort();

    const int month_ = to_int(month, ok), day_ = to_int(day, ok);

    (void) to_int(year, ok);
    (void) to_int(serial, ok);

    if (!ok)
        abort();

    if (month_ < 1 || month_ > 12)
        abort();

    if (day_ < 1 || day_ > 31)
        abort();

    migrations().push_back(m);
}

std::vector<migration*>& migrator::migrations()
{
    static std::vector<migration*> ret;
    return ret;
}

QString migrator::last_migration_time()
{
    QString ret;

    std::shared_ptr<QSettings> s(options::group::ini_file());

    s->beginGroup("migrations");
    ret = s->value("last-migration-at", "19700101_00").toString();
    s->endGroup();

    return ret;
}

QString migrator::time_after_migrations()
{
    const std::vector<migration*> list = sorted_migrations();

    if (list.size() == 0u)
        return QStringLiteral("19700101_00");

    QString ret = list[list.size() - 1]->unique_date();
    ret += QStringLiteral("~");

    return ret;
}

void migrator::set_last_migration_time(const QString& val)
{
    std::shared_ptr<QSettings> s(options::group::ini_file());

    s->beginGroup("migrations");
    s->setValue("last-migration-at", val);
    s->endGroup();
}

std::vector<migration*> migrator::sorted_migrations()
{
    std::vector<migration*> list(migrations());

    using mm = migration*;

    std::sort(list.begin(), list.end(), [](const mm x, const mm y) { return x->unique_date() < y->unique_date(); });
    return list;
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
    std::vector<migration*> migrations = sorted_migrations();
    std::vector<QString> done;

    const QString last_migration = last_migration_time();

    for (migration* m_ : migrations)
    {
        migration& m(*m_);

        const QString date = m.unique_date();

        if (date <= last_migration)
            continue;

        if (m.should_run())
        {
            m.run();
            done.push_back(m.name());
        }
    }

    mark_config_as_not_needing_migration();

    if (done.size())
    {
        for (const QString& name : done)
        {
            const QByteArray data = name.toUtf8();
            qDebug() << "migrate:" << data.constData();
        }
    }

    return done;
}

}

migration::migration() {}
migration::~migration() {}

} // ns

std::vector<QString> run_migrations()
{
    return migrations::detail::migrator::run();
}

void mark_config_as_not_needing_migration()
{
    using m = migrations::detail::migrator;

    m::mark_config_as_not_needing_migration();
}

void migrations::detail::migrator::mark_config_as_not_needing_migration()
{
    set_last_migration_time(time_after_migrations());
}
