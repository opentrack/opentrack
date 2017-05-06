/* Copyright (c) 2014-2015 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "export.hpp"
#include "options/options.hpp"
using namespace options;
#include "spline/spline.hpp"
#include "main-settings.hpp"

struct OTR_LOGIC_EXPORT Map final
{
    Map(QString primary, QString secondary, int max_x, int max_y, axis_opts& opts);

    void save();
    void load();

    axis_opts& opts;
    QString name1, name2;
    spline spline_main, spline_alt;
};

class OTR_LOGIC_EXPORT Mappings final
{
private:
    Map axes[6];
public:
    Mappings(std::vector<axis_opts*> opts);

    Map& operator()(int i) { return axes[i]; }
    const Map& operator()(int i) const { return axes[i]; }
    Map& operator()(unsigned i) { return axes[i]; }
    const Map& operator()(unsigned  i) const { return axes[i]; }

    template<typename f> void forall(f&& fun)
    {
        for (unsigned i = 0; i < 6; i++)
            fun(axes[i]);
    }
};
