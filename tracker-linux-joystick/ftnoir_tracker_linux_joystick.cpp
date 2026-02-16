/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "ftnoir_tracker_linux_joystick.h"
#include "api/plugin-api.hpp"
#include "compat/math.hpp"
#include <QMutexLocker>

joystick::joystick()
{
    QString device = getJoystickDevice(s.guid);
    joy_fd = open(device.toUtf8().data(), O_RDONLY | O_NONBLOCK);
}


joystick::~joystick() {
    if (joy_fd > 0) close(joy_fd);
}

module_status joystick::start_tracker(QFrame *)
{
    if (joy_fd == -1) return error("Couldn't open joystick");
    return status_ok();
}


void joystick::data(double *data)
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
    int axes[JOY_CHANNELS];
    struct js_event event;
    bool ret = true;
    if (read(joy_fd, &event, sizeof(event)) > 0)
    {
        switch (event.type)
        {
        case JS_EVENT_AXIS:
            if (event.number >= JOY_CHANNELS) break;
            axes_state[event.number] = event.value;
            break;
        default:
            /* Ignore init/button events. */
            break;
        }
    }

    for (int i = 0; i < JOY_CHANNELS; i++)
    {
        axes[i] = axes_state[i];
    }
    if (ret)
    {
        for (int i = 0; i < 6; i++)
        {
            int k = map[i];
            if (k < 0 || k >= JOY_CHANNELS)
                data[i] = 0;
            else
                data[i] = std::clamp(axes[k] * limits[i] / AXIS_MAX,
                                     -limits[i], limits[i]);
        }
    }
}

OPENTRACK_DECLARE_TRACKER(joystick, dialog_joystick, joystickDll)
