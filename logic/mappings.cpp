#include "mappings.hpp"

#include <utility>

Map::Map(const QString& spline_name, const QString& alt_spline_name, axis_opts& opts) :
    opts(opts),
    name(spline_name),
    alt_name(alt_spline_name),
    spline_main(spline_name, opts.prefix(), opts.axis()),
    spline_alt(alt_spline_name, opts.prefix(), opts.axis())
{
}

void Map::save()
{
    spline_main.save();
    spline_alt.save();
}

void Map::load()
{
    spline_main.reload();
    spline_alt.reload();
}


Mappings::Mappings(axis_opts_impl::axis_opts** opts) :
    axes {
        { "spline-X",       "alt-spline-X",    *opts[TX] },
        { "spline-Y",       "alt-spline-Y",    *opts[TY] },
        { "spline-Z",       "alt-spline-Z",    *opts[TZ] },
        { "spline-yaw",     "alt-spline-yaw",  *opts[Yaw] },
        { "spline-pitch",   "alt-spline-pitch",*opts[Pitch] },
        { "spline-roll",    "alt-spline-roll", *opts[Roll] }
    }
{}
