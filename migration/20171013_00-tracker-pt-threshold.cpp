/* Copyright (c) 2017, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "migration.hpp"
#include "options/options.hpp"
#include "compat/util.hpp"
//#include "tracker-pt/ftnoir_tracker_pt_settings.h"

using namespace options;
using namespace migrations;

static constexpr const char* old_name = "threshold-primary";
static constexpr const char* new_name = "threshold-slider";
static constexpr const char* bundle_name = "tracker-pt";

struct move_int_to_slider : migration
{
    QString unique_date() const override
    {
        return "20171013_00";
    }


    QString name() const override
    {
        return "tracker/pt threshold slider (int -> slider_value)";
    }

    bool should_run() const override
    {
        bundle b = make_bundle("tracker-pt");

        return b->contains(old_name) && !b->contains(new_name);
    }

    void run() override
    {
        bundle b = make_bundle("tracker-pt");

        value<int> old_val(b, old_name, 128);
        value<slider_value> new_val(b, new_name, slider_value(128, 0, 255));

        new_val = slider_value(old_val.to<int>(), 0, 255);

        b->save();
    }
};

OPENTRACK_MIGRATION(move_int_to_slider);
