#pragma once

#include "options/options.hpp"
using namespace options;
#include "spline-widget/spline.hpp"

struct settings_accela : opts
{
    static constexpr double rot_gains[16][2] =
    {
        { 12, 500 },
        { 11, 450 },
        { 10, 400 },
        { 9, 350 },
        { 8, 300 },
        { 7, 250 },
        { 6, 200 },
        { 2.66, 50 },
        { 1.66, 17 },
        { 1, 4 },
        { .5, .53 },
        { 0, 0 },
        { -1, 0 }
    };

    static constexpr double trans_gains[16][2] =
    {
        { 12, 400 },
        { 11, 350 },
        { 10, 300 },
        { 9, 250 },
        { 8, 200 },
        { 7, 150 },
        { 5, 80 },
        { 3, 32 },
        { 2, 10 },
        { 1.66, 6 },
        { 1.33, 3 },
        { .66, 1 },
        { .33, .5 },
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
        rot_sensitivity(b, "rotation-sensitivity", slider_value(1.5, .1, 2)),
        pos_sensitivity(b, "translation-sensitivity", slider_value(1., .05, 1.5)),
        rot_deadzone(b, "rotation-deadzone", slider_value(.03, 0, .1)),
        pos_deadzone(b, "translation-deadzone", slider_value(.1, 0, 1)),
        ewma(b, "ewma", slider_value(0, 0, 15)),
        rot_nonlinearity(b, "rotation-nonlinearity", slider_value(1.2, 1, 1.3))
    {}
};
