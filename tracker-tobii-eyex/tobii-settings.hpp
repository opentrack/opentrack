#pragma once

#include "options/options.hpp"
using namespace options;

enum tobii_mode
{
    tobii_snap = 0xf00d,
    tobii_acc = 0xacc,
};

enum max_yaw
{
    y10, y15, y20, y30, y45,
};

enum max_pitch
{
    p10, p15, p25, p35,
};

struct settings final : public opts
{
    value<tobii_mode> mode { b, "mode", tobii_snap };

    value<slider_value> snap_speed {b, "snap-speed", slider_value(.1, .05, 1)},
                        snap_inv_dz {b, "snap-screen-edge-length", slider_value(.35, .1, .5)};
    value<slider_value> acc_speed {b, "acc-speed", slider_value(.1, .05, 1)},
                        acc_dz_len {b, "acc-screen-edge-length", slider_value(.1, .1, 1)};
    value<max_yaw> snap_yaw {b, "snap-max-yaw", y20},
                   acc_yaw {b, "acc-max-yaw", y20};
    value<max_pitch> snap_pitch {b, "snap-max-pitch", p15},
                     acc_pitch {b, "acc-max-pitch", p15};

    settings() : opts("tobii-eyex") {}
};
