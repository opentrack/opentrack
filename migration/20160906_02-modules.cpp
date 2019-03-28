/* Copyright (c) 2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "migration.hpp"
#include "options/options.hpp"

#include <QString>
#include <QVariant>

using namespace options;
using namespace migrations;

static const char* const module_names[3] =
{
    "tracker-dll",
    "protocol-dll",
    "filter-dll",
};

struct split_modules_rc11 : migration
{
    split_modules_rc11() = default;

    QString unique_date() const override
    {
        return "20160909_02";
    }

    QString name() const override
    {
        return "split modules to their own section";
    }

    bool should_run() const override
    {
        bundle new_bundle = make_bundle("modules");
        bundle old_bundle = make_bundle("opentrack-ui");

        for (const char* name : module_names)
            if (new_bundle->contains(name))
                return false;

        for (const char* name : module_names)
            if (old_bundle->contains(name))
                return true;

        return false;
    }

    void run() override
    {
        bundle new_bundle = make_bundle("modules");
        bundle old_bundle = make_bundle("opentrack-ui");

        for (const char* name : module_names)
            new_bundle->store_kv(name, old_bundle->get_variant(name));

        new_bundle->save();
    }
};

OPENTRACK_MIGRATION(split_modules_rc11)
