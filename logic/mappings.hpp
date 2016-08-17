/* Copyright (c) 2014-2015 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "options/options.hpp"
using namespace options;
#include "spline-widget/spline.hpp"
#include "main-settings.hpp"

struct Map final
{
    Map(QString primary,
        QString secondary,
        int max_x,
        int max_y,
        axis_opts& opts) :
        opts(opts),
        name1(primary),
        name2(secondary),
        spline_main(max_x, max_y, primary),
        spline_alt(max_x, max_y, secondary)
    {
    }

    void save(QSettings& s)
    {
        spline_main.save(s);
        spline_alt.save(s);
    }

    void load()
    {
        spline_main.reload();
        spline_alt.reload();
    }

    axis_opts& opts;
    QString name1, name2;
    spline spline_main, spline_alt;
};

class Mappings
{
private:
    Map axes[6];
public:
    Mappings(std::vector<axis_opts*> opts) :
        axes {
            Map("spline-X", "alt-spline-X", 30, 75, *opts[TX]),
            Map("spline-Y", "alt-spline-Y", 30, 75, *opts[TY]),
            Map("spline-Z", "alt-spline-Z", 30, 75, *opts[TZ]),
            Map("spline-yaw", "alt-spline-yaw", 180, 180, *opts[Yaw]),
            Map("spline-pitch", "alt-spline-pitch", 180, 180, *opts[Pitch]),
            Map("spline-roll", "alt-spline-roll", 180, 180, *opts[Roll])
        }
    {}

    inline Map& operator()(int i) { return axes[i]; }
    inline const Map& operator()(int i) const { return axes[i]; }
    inline Map& operator()(unsigned i) { return axes[i]; }
    inline const Map& operator()(unsigned  i) const { return axes[i]; }

    template<typename f> void forall(f&& fun)
    {
        for (unsigned i = 0; i < 6; i++)
        {
            fun(axes[i]);
        }
    }
};
