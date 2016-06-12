/* Copyright (c) 2014, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "test.h"
#include "opentrack/plugin-api.hpp"
#include <cmath>

#include <QDebug>

const double FTNoIR_Tracker::incr[6] =
{
    2, 3, 4,
    70, 5, 3
};

FTNoIR_Tracker::FTNoIR_Tracker() :
    last_x { 0, 0, 0, 0, 0, 0 }
{
}

FTNoIR_Tracker::~FTNoIR_Tracker()
{
}

void FTNoIR_Tracker::start_tracker(QFrame*)
{
    t.start();
}

void FTNoIR_Tracker::data(double *data)
{
    using std::fmod;
    using std::sin;
    using std::fabs;
    using std::copysign;

    const double dt = t.elapsed_seconds();
    t.start();

    for (int i = 0; i < 6; i++)
    {
        double x = last_x[i] + incr[i] * d2r * dt;

        x = fmod(x , 2 * pi);

        last_x[i] = x;

        if (x > pi + pi/2)
        {
            x -= pi;
        }

        double ret = sin(x) * 180;

        data[i] = ret;
    }
}

TrackerControls::TrackerControls()
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
}

void TrackerControls::doOK()
{
    //s.b->save();
    close();
}

void TrackerControls::doCancel()
{
    close();
}

OPENTRACK_DECLARE_TRACKER(FTNoIR_Tracker, TrackerControls, FTNoIR_TrackerDll)
