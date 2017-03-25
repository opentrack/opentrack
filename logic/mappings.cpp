#pragma once

#include "mappings.hpp"

Map::Map(QString primary, QString secondary, int max_x, int max_y, axis_opts& opts) :
    opts(opts),
    name1(primary),
    name2(secondary),
    spline_main(max_x, max_y, primary),
    spline_alt(max_x, max_y, secondary)
{
    spline_main.set_max_input(opts.clamp);
    spline_alt.set_max_input(opts.clamp);
}

void Map::save(QSettings& s)
{
    spline_main.save(s);
    spline_alt.save(s);
}

void Map::load()
{
    spline_main.reload();
    spline_alt.reload();
}

Mappings::Mappings(std::vector<axis_opts*> opts) :
    axes {
        Map("spline-X", "alt-spline-X", 30, 75, *opts[TX]),
        Map("spline-Y", "alt-spline-Y", 30, 75, *opts[TY]),
        Map("spline-Z", "alt-spline-Z", 30, 75, *opts[TZ]),
        Map("spline-yaw", "alt-spline-yaw", 180, 180, *opts[Yaw]),
        Map("spline-pitch", "alt-spline-pitch", 180, 180, *opts[Pitch]),
        Map("spline-roll", "alt-spline-roll", 180, 180, *opts[Roll])
        }
{}
