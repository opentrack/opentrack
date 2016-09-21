/* Copyright (c) 2014, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "test.h"
#include "api/plugin-api.hpp"
#include <cmath>

#include <QDebug>

const double FTNoIR_Tracker::incr[6] =
{
    50, 40, 80,
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

#ifdef EMIT_NAN
#   include <cstdlib>
#endif

void FTNoIR_Tracker::data(double *data)
{
    using std::fmod;
    using std::sin;
    using std::fabs;
    using std::copysign;

    const double dt = t.elapsed_seconds();
    t.start();

#ifdef EMIT_NAN
    if ((rand()%4) == 0)
    {
        for (int i = 0; i < 6; i++)
            data[i] = 0./0.;
    }
    else
#endif
        for (int i = 0; i < 6; i++)
        {
            double x = fmod(last_x[i] + incr[i] * d2r * dt, 2 * M_PI);
            last_x[i] = x;

            if (i >= 3)
            {
#ifdef DISCONTINUITY
                if (x > pi + pi/2)
                    x -= M_PI;
                else if (x > pi/2 && x < pi)
                    x += M_PI;
#endif

                data[i] = sin(x) * 180;
            }
            else
            {
                data[i] = sin(x) * 100;
            }
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
