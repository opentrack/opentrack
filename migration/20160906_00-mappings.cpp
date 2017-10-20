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

using namespace migrations;

struct mappings_from_2_3_0_rc11 : migration
{
    static QList<QList<QPointF>> get_old_splines()
    {
        QList<QList<QPointF>> ret;

        static const char* names[] =
        {
            "tx", "tx_alt",
            "ty", "ty_alt",
            "tz", "tz_alt",
            "rx", "rx_alt",
            "ry", "ry_alt",
            "rz", "rz_alt",
        };

        return group::with_settings_object([&](QSettings& settings) {
            for (const char* name : names)
            {
                QList<QPointF> points;

                settings.beginGroup(QString("Curves-%1").arg(name));

                const int max = settings.value("point-count", 0).toInt();

                for (int i = 0; i < max; i++)
                {
                    QPointF new_point(settings.value(QString("point-%1-x").arg(i), 0).toDouble(),
                                      settings.value(QString("point-%1-y").arg(i), 0).toDouble());

                    points.append(new_point);
                }

                settings.endGroup();

                ret.append(points);
            }

            return ret;
        });
    }

    QString unique_date() const override { return "20160909_00"; }
    QString name() const override { return "mappings to new layout"; }

    static Mappings get_new_mappings()
    {
        main_settings s;
        return Mappings(s.all_axis_opts);
    }

    bool should_run() const override
    {
        Mappings m = get_new_mappings();

        // run only if no new splines were set
        for (int i = 0; i < 6; i++)
            if (m(i).spline_main.get_point_count() || m(i).spline_alt.get_point_count())
                return false;

        // run only if old splines exist
        for (const QList<QPointF>& points : get_old_splines())
            if (points.size())
                return true;

        // no splines exit at all
        return false;
    }
    void run() override
    {
        const QList<QList<QPointF>> old_mappings = get_old_splines();
        Mappings m = get_new_mappings();

        for (int i = 0; i < 12; i++)
        {
            spline& spl = (i % 2) == 0 ? m(i / 2).spline_main : m(i / 2).spline_alt;
            spl.clear();
            const QList<QPointF>& points = old_mappings[i];
            for (const QPointF& pt : points)
                spl.add_point(pt);
            spl.save();
        }
    }
};

OPENTRACK_MIGRATION(mappings_from_2_3_0_rc11);
