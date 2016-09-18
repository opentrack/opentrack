/* Copyright (c) 2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <QString>
#include <vector>

#include "export.hpp"

namespace migrations {

class migration;
class registrator;

namespace detail {
    class migrator final
    {
        static std::vector<migration*>& migrations();
        static QString last_migration_time();
        static QString time_after_migrations();
        static void set_last_migration_time(const QString& val);
        migrator() = delete;
        static std::vector<migration*> sorted_migrations();
        static int to_int(const QString& str, bool& ok);
    public:
        static std::vector<QString> run();
        static void register_migration(migration* m);
        static void mark_config_as_not_needing_migration();
    };

    template<typename t>
    struct registrator final
    {
        registrator()
        {
            static t m;
            migrator::register_migration(static_cast<migration*>(&m));
        }
    };
}

#ifndef __COUNTER__
#   error "oops, need __COUNTER__ extension for preprocessor"
#endif

#define OPENTRACK_MIGRATION(type) static ::migrations::detail::registrator<type> opentrack_migration_registrator__ ## __COUNTER__ ## _gensym

#ifdef Q_CREATOR_RUN
#   pragma clang diagnostic ignored "-Wweak-vtables"
#endif

class migration
{
    migration& operator=(const migration&) = delete;
    migration(const migration&) = delete;
    migration& operator=(migration&&) = delete;
    migration(migration&&) = delete;

public:
    migration();
    virtual ~migration();
    virtual QString unique_date() const = 0;
    virtual QString name() const = 0;
    virtual bool should_run() const = 0;
    virtual void run() = 0;
};

}

OPENTRACK_MIGRATION_EXPORT std::vector<QString> run_migrations();
OPENTRACK_MIGRATION_EXPORT void mark_config_as_not_needing_migration();
