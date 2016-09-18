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

struct split_modules_rc11 : migration
{
    split_modules_rc11() = default;

    static const char* names[3];

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

        for (const char* name : names)
            if (new_bundle->contains(name))
                return false;

        for (const char* name : names)
            if (old_bundle->contains(name))
                return true;

        return false;
    }

    void run() override
    {
        bundle new_bundle = make_bundle("modules");
        bundle old_bundle = make_bundle("opentrack-ui");

        for (const char* name : names)
            new_bundle->store_kv(name, QVariant(old_bundle->get<QString>(name)));

        new_bundle->save();
    }
};

const char* split_modules_rc11::names[3] =
{
    "tracker-dll",
    "protocol-dll",
    "filter-dll",
};

OPENTRACK_MIGRATION(split_modules_rc11);
