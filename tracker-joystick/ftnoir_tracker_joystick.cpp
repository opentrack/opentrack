/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "ftnoir_tracker_joystick.h"
#include "opentrack/plugin-api.hpp"
#include <QMutexLocker>

FTNoIR_Tracker::FTNoIR_Tracker()
{
    if (static_cast<QString>(s.guid) == "")
    {
        std::vector<win32_joy_ctx::joy_info> info = joy_ctx.get_joy_info();
        if (info.size())
        {
            s.guid = info[0].guid;
            s.b->save();
        }
    }
}

FTNoIR_Tracker::~FTNoIR_Tracker()
{
}

void FTNoIR_Tracker::start_tracker(QFrame*)
{
}

void FTNoIR_Tracker::data(double *data)
{
    int map[6] = {
        s.joy_1 - 1,
        s.joy_2 - 1,
        s.joy_3 - 1,
        s.joy_4 - 1,
        s.joy_5 - 1,
        s.joy_6 - 1,
    };

    const double limits[] = {
        100,
        100,
        100,
        180,
        180,
        180
    };
    
    const QString guid = s.guid;
    int axes[8];
    const bool ret = joy_ctx.poll_axis(guid, axes);
    
    if (ret)
    {
        for (int i = 0; i < 6; i++)
        {
            int k = map[i] - 1;
            if (k < 0 || k >= 8)
                data[i] = 0;
            else
                data[i] = axes[k] * limits[i] / AXIS_MAX;
        }
    }
}

OPENTRACK_DECLARE_TRACKER(FTNoIR_Tracker, TrackerControls, FTNoIR_TrackerDll)
