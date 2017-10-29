/* Copyright (c) 2017, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "migration.hpp"
#include "options/options.hpp"
#include "logic/mappings.hpp"
#include "logic/main-settings.hpp"

#include <QString>

using namespace options;
using namespace migrations;

struct max_pitch_output : migration
{
    max_pitch_output() = default;

    QString unique_date() const override
    {
        return "20171020_00";
    }

    QString name() const override
    {
        return "max pitch output in mapping window";
    }

    bool should_run() const override
    {
        {
            constexpr char const* name = "pitch-max-output-value";

            bundle b = make_bundle("opentrack-mappings");

            if (b->contains(name))
                return false;
        }

        main_settings s;
        Mappings m { s.all_axis_opts };

        Map& pitch_map = m(Pitch);
        spline& pitch_spline_1 = pitch_map.spline_main;
        spline& pitch_spline_2 = pitch_map.spline_alt;

        for (const spline& spl : { pitch_spline_1, pitch_spline_2 })
            for (QPointF& point : spl.get_points())
                if (point.y() - 1e-2 > 90)
                    return true;

        return false;
    }

    void run() override
    {
        main_settings s;
        axis_opts& pitch_opts = s.a_pitch;

        pitch_opts.clamp_y_ = axis_opts::o_r180;

        s.b_map->save();
    }
};

OPENTRACK_MIGRATION(max_pitch_output);
