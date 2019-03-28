/* Copyright (c) 2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "migration.hpp"
#include "logic/mappings.hpp"
#include "logic/main-settings.hpp"
#include "options/group.hpp"

#include <QPointF>
#include <QList>

#include <memory>

#include <QDebug>

using namespace options;
using namespace options::globals;
using namespace migrations;

static const char* const old_names[] =
{
    "tx", "tx_alt",
    "ty", "ty_alt",
    "tz", "tz_alt",
    "rx", "rx_alt",
    "ry", "ry_alt",
    "rz", "rz_alt",
};

static const char* const new_names[] = {
    "spline-X",       "alt-spline-X",
    "spline-Y",       "alt-spline-Y",
    "spline-Z",       "alt-spline-Z",
    "spline-yaw",     "alt-spline-yaw",
    "spline-pitch",   "alt-spline-pitch",
    "spline-roll",    "alt-spline-roll",
};

static QList<QList<QPointF>> get_old_splines()
{
    QList<QList<QPointF>> ret;

    return with_settings_object([&](QSettings& settings) {
        for (const char* name : old_names)
        {
            const int max = settings.value("point-count", 0).toInt();

            if (max < 0 || max > 1 << 16)
                return ret;

            QList<QPointF> points;
            points.reserve(max);

            settings.beginGroup(QString("Curves-%1").arg(name));

            for (int i = 0; i < max; i++)
                points.append({ settings.value(QString("point-%1-x").arg(i), 0).toDouble(),
                                settings.value(QString("point-%1-y").arg(i), 0).toDouble() });

            settings.endGroup();

            ret.append(points);
        }

        return ret;
    });
}

struct mappings_from_2_3_0_rc11 : migration
{
    QString unique_date() const override { return "20160909_00"; }
    QString name() const override { return "mappings to new layout"; }

    bool should_run() const override
    {        
        for (const char* name : new_names)
        {
            // run only if no new splines were set
            auto b = make_bundle(name);
            if (b->contains("points"))
                return false;

            // run only if old splines exist
            for (const QList<QPointF>& points : get_old_splines())
                if (!points.empty())
                    return true;
        }

        // no splines exit at all
        return false;
    }

    void run() override
    {        
        with_settings_object([](QSettings&) {
            const QList<QList<QPointF>> old_mappings = get_old_splines();

            for (int i = 0; i < 12; i++)
            {
                auto b = make_bundle(new_names[i]);
                if (b->contains("points"))
                    continue;
                value<QList<QPointF>> new_value { b, "points", {} };
                new_value = old_mappings[i];
                b->save();
            }
        });
    }
};

OPENTRACK_MIGRATION(mappings_from_2_3_0_rc11)
