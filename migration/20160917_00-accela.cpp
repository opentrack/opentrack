/* Copyright (c) 2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "migration.hpp"
#include "options/options.hpp"
#include "compat/util.hpp"

#include "filter-accela/accela-settings.hpp"

using namespace migrations;
using namespace options;

struct move_accela_to_sliders : migration
{
    struct map
    {
        const char* old_name;
        double old_max_100;
        double old_min;
        value<slider_value>* new_slider;
    };

    static constexpr const char* old_bundle_name = "Accela";
    static constexpr const char* new_bundle_name = "accela-sliders";
    static constexpr const char* slider_name = "rotation-nonlinearity";

    struct map_ { map s[8]; };

    static map_ make_settings(settings_accela& s)
    {
        map_ ret
        {
            {
                { "rotation-threshold", 4, 1, &s.rot_sensitivity },
                { "translation-threshold", 4, 1, &s.pos_sensitivity },
                { "rotation-deadzone", 4, 0, &s.rot_deadzone },
                { "translation-deadzone", 4, 0, &s.pos_deadzone },
                { "ewma", 1.25, 0, &s.ewma },
                { nullptr, 0, 0, nullptr },
            }
        };
        return ret;
    }

    move_accela_to_sliders() = default;

    QString unique_date() const override { return "20160917_00"; }
    QString name() const override { return "move accela to .ini sliders"; }

    bool should_run() const override
    {
        settings_accela s;
        map_ ss = make_settings(s);
        map* settings = ss.s;

        const bundle old_b = make_bundle(old_bundle_name);
        const bundle new_b = make_bundle(new_bundle_name);

        bool old_found = false;

        for (unsigned i = 0; settings[i].old_name; i++)
        {
            const map& cur = settings[i];
            if (new_b->contains(cur.new_slider->name()))
                return false;
            if (old_b->contains(cur.old_name))
                old_found = true;
        }

        old_found |= old_b->contains(slider_name);
        old_found &= !new_b->contains(slider_name);

        return old_found;
    }

    void run() override
    {
        settings_accela s;
        map_ ss = make_settings(s);
        map* settings = ss.s;

        bundle old_b = make_bundle(old_bundle_name);
        bundle new_b = make_bundle(new_bundle_name);

        for (unsigned i = 0; settings[i].old_name; i++)
        {
            const map& cur = settings[i];

            const slider_value val = progn(
                if (old_b->contains(cur.old_name))
                {
                    const double old = old_b->get<double>(cur.old_name);
                    return slider_value((cur.old_min + old) * cur.old_max_100 / 100.,
                                        cur.new_slider->default_value().min(),
                                        cur.new_slider->default_value().max());
                }
                else
                    return cur.new_slider->default_value();
            );

            value<slider_value> tmp(new_b, cur.new_slider->name(), slider_value(-1e6, val.min(), val.max()));
            tmp = val;
        }

        new_b->save();
    }
};

// odr
constexpr settings_accela::gains settings_accela::rot_gains[16];
constexpr settings_accela::gains settings_accela::pos_gains[16];

constexpr const char* move_accela_to_sliders::old_bundle_name;
constexpr const char* move_accela_to_sliders::new_bundle_name;
constexpr const char* move_accela_to_sliders::slider_name;

OPENTRACK_MIGRATION(move_accela_to_sliders);
