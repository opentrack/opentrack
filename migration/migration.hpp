/* Copyright (c) 2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <QString>
#include "export.hpp"

#include <memory>
#include <vector>
#include <functional>

namespace migrations {

struct migration;
class registrator;

namespace detail {
    using mptr = std::shared_ptr<migration>;
    using mfun = std::function<mptr ()>;

    struct migrator
    {
        static std::vector<QString> run();
        static void add_migration_thunk(std::function<mptr()>& thunk);
        static void mark_profile_as_not_needing_migration();

    private:
        static void sort_migrations();

        static void register_migration(const mptr& m);
        static std::vector<mptr>& migrations();

        static void eval_thunks();

        static QString last_migration_time();
        static QString time_after_migrations();

        static void set_last_migration_time(const QString& val);
        static int to_int(const QString& str, bool& ok);

        static std::vector<mptr>& migration_list();
        static std::vector<mfun>& migration_thunks();
    };

    template<typename t>
    struct registrator final
    {
        registrator()
        {
            mfun f { [] { return std::shared_ptr<migration>(new t); } };

            migrator::add_migration_thunk(f);
        }
    };
}

#define OPENTRACK_MIGRATION3(type, ctr) \
    static const char init_##ctr = (::migrations::detail::registrator<type>{}, 0);

#define OPENTRACK_MIGRATION2(type, ctr) \
    OPENTRACK_MIGRATION3(type, ctr)

#define OPENTRACK_MIGRATION(type) \
    OPENTRACK_MIGRATION2(type, __COUNTER__)

struct migration
{
    migration();
    migration(const migration&) = delete;
    migration& operator=(const migration&) = delete;

    virtual ~migration();
    virtual QString unique_date() const = 0;
    virtual QString name() const = 0;
    virtual bool should_run() const = 0;
    virtual void run() = 0;
};

} // ns migrations

OTR_MIGRATION_EXPORT std::vector<QString> run_migrations();
OTR_MIGRATION_EXPORT void mark_profile_as_not_needing_migration();
