/* Copyright (c) 2014-2015 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include <QSettings>
#include "opentrack-compat/options.hpp"
using namespace options;
#include "spline-widget/spline.hpp"
#include "main-settings.hpp"

class Map {
public:
    Map(QString primary,
            QString secondary,
            int max_x,
            int max_y,
            axis_opts& opts) :
        spline_main(max_x, max_y),
        spline_alt(max_x, max_y),
        opts(opts),
        name1(primary),
        name2(secondary)
    {
        mem<QSettings> iniFile = group::ini_file();
        spline_main.loadSettings(*iniFile, primary);
        spline_alt.loadSettings(*iniFile, secondary);
    }
    spline spline_main;
    spline spline_alt;
    axis_opts& opts;
    QString name1, name2;
};

class Mappings {
private:
    Map axes[6];
public:
    Mappings(std::vector<axis_opts*> opts) :
        axes {
            Map("tx","tx_alt", 30, 75, *opts[TX]),
            Map("ty","ty_alt", 30, 75, *opts[TY]),
            Map("tz","tz_alt", 30, 75, *opts[TZ]),
            Map("rx", "rx_alt", 180, 180, *opts[Yaw]),
            Map("ry", "ry_alt", 180, 180, *opts[Pitch]),
            Map("rz", "rz_alt", 180, 180, *opts[Roll])
        }
    {}

    inline Map& operator()(int i) { return axes[i]; }
    inline const Map& operator()(int i) const { return axes[i]; }

    void load_mappings()
    {
        mem<QSettings> iniFile = group::ini_file();

        for (int i = 0; i < 6; i++)
        {
            axes[i].spline_main.loadSettings(*iniFile, axes[i].name1);
            axes[i].spline_alt.loadSettings(*iniFile, axes[i].name2);
            axes[i].opts.b->reload();
        }
    }
    void save_mappings()
    {
        mem<QSettings> iniFile = group::ini_file();

        for (int i = 0; i < 6; i++)
        {
            axes[i].spline_main.saveSettings(*iniFile, axes[i].name1);
            axes[i].spline_alt.saveSettings(*iniFile, axes[i].name2);
            axes[i].opts.b->save();
        }
    }

    void invalidate_unsaved()
    {
        for (int i = 0; i < 6; i++)
        {
            axes[i].spline_main.invalidate_unsaved_settings();
            axes[i].spline_alt.invalidate_unsaved_settings();
            axes[i].opts.b->reload();
        }
    }
};
