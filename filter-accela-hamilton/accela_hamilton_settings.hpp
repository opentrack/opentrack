#pragma once

#include "spline/spline.hpp"
#include "options/options.hpp"

namespace detail::accela_hamilton {

using namespace options;

// ------------------------------------
// debug knobs
// ------------------------------------

//#define DEBUG_ACCELA
//#define SPLINE_ROT_DEBUG
//#define SPLINE_TRANS_DEBUG

struct settings_accela_hamilton : opts
{
    struct gains
    {
        double x, y;
    };

    static constexpr gains const rot_gains[] {
        { 9, 300 },
        { 8, 200 },
        { 5, 100 },
        { 2.5, 35 },
        { 1.5, 8 },
        { 1, 1.5 },
        { .5, .4 },
    };

    static constexpr gains const pos_gains[] {
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
    };

    static void make_splines(spline& rot, spline& pos);

    value<slider_value> rot_smoothing { b, "rotation-sensitivity", { 1.5, .05, 2.5 } },
                        pos_smoothing { b, "translation-sensitivity", { 1., .05, 1.5 } },
                        rot_deadzone { b, "rotation-deadzone", { .03, 0, .2 } },
                        pos_deadzone { b, "translation-deadzone", { .1, 0, 1 } },
                        max_zoomed_smoothing { b, "max_zoomed_smoothing",  { .0, .0, 10. } },
                        max_z { b, "max-z", { 10., .0, 30. } };

    settings_accela_hamilton() : opts("accela-hamilton-sliders") {}
};

} // ns detail::accela_hamilton

using detail::accela_hamilton::settings_accela_hamilton;
