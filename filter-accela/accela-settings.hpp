#pragma once

#include "options/options.hpp"
using namespace options;
#include "spline/spline.hpp"

struct settings_accela : opts
{
    static constexpr double rot_gains[16][2] =
    {
        { 8, 700 },
        { 7, 300 },
        { 6, 160 },
        { 5, 95 },

        { 4, 55 },
        { 3, 25 },
        { 1.66, 10 },
        { 1, 4 },
        { .5, .53 },
        { 0, 0 },
        { -1, 0 }
    };

    static constexpr double pos_gains[16][2] =
    {
        { 9, 200 },
        { 8, 150 },
        { 7, 110 },
        { 5, 60 },
        { 3, 24 },
        { 2, 7.5 },
        { 1.66, 4.5 },
        { 1.33, 2.25 },
        { .66, .75 },
        { .33, .375 },
        { 0, 0 },
        { -1, 0 }
    };

    static void make_splines(spline& rot, spline& trans);

    value<slider_value> rot_sensitivity, pos_sensitivity;
    value<slider_value> rot_deadzone, pos_deadzone;
    value<slider_value> ewma;
    value<slider_value> rot_nonlinearity;
    settings_accela() :
        opts("accela-sliders"),
        rot_sensitivity(b, "rotation-sensitivity", slider_value(1.5, .5, 3)),
        pos_sensitivity(b, "translation-sensitivity", slider_value(1., .05, 1.5)),
        rot_deadzone(b, "rotation-deadzone", slider_value(.03, 0, .1)),
        pos_deadzone(b, "translation-deadzone", slider_value(.1, 0, 1)),
        ewma(b, "ewma", slider_value(0, 0, 300)),
        rot_nonlinearity(b, "rotation-nonlinearity", slider_value(1.2, 1, 1.3))
    {}
};
