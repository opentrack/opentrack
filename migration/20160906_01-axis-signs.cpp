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

const char* const axis_names[] =
{
    "yaw", "pitch", "roll",
    "x", "y", "z",
};

const QString alt_sign_fmt = QStringLiteral("%1-alt-axis-sign");

struct axis_signs_split_rc11 : migration
{
    axis_signs_split_rc11() = default;

    QString unique_date() const override
    {
        return "20160909_01";
    }

    QString name() const override
    {
        return "asymmetric axis option to other section";
    }

    bool should_run() const override;
    void run() override;
};

OPENTRACK_MIGRATION(axis_signs_split_rc11)

bool axis_signs_split_rc11::should_run() const
{
    bundle new_bundle = make_bundle("opentrack-mappings");
    bundle old_bundle = make_bundle("opentrack-ui");

    for (const char* name : axis_names)
    {
        // new present, already applied
        if (new_bundle->contains(alt_sign_fmt.arg(name)))
            return false;
    }

    for (const char* name : axis_names)
    {
        // old present
        if (old_bundle->contains(alt_sign_fmt.arg(name)))
            return true;
    }

    // nothing to copy
    return false;
}

void axis_signs_split_rc11::run()
{
    bundle new_bundle = make_bundle("opentrack-mappings");
    bundle old_bundle = make_bundle("opentrack-ui");

    for (const char* name : axis_names)
        new_bundle->store_kv(alt_sign_fmt.arg(name),
                             old_bundle->get_variant(alt_sign_fmt.arg(name)));

    new_bundle->save();
}
