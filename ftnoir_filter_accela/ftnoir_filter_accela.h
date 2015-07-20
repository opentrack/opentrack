/* Copyright (c) 2012-2015 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once
#include "opentrack/plugin-api.hpp"
#include "qfunctionconfigurator/functionconfig.h"
#include <atomic>
#include <QMutex>
#include <QTimer>

#include "opentrack/options.hpp"
using namespace options;
#include "opentrack/timer.hpp"

struct settings_accela : opts {
    value<int> rot_threshold, trans_threshold, ewma, rot_deadzone, trans_deadzone;
    static constexpr double mult_rot = 4. / 100.;
    static constexpr double mult_trans = 4. / 100.;
    static constexpr double mult_rot_dz = 2. / 100.;
    static constexpr double mult_trans_dz = 2. / 100.;
    static constexpr double mult_ewma = 1.25;
    settings_accela() :
        opts("Accela"),
        rot_threshold(b, "rotation-threshold", 30),
        trans_threshold(b, "translation-threshold", 50),
        ewma(b, "ewma", 2),
        rot_deadzone(b, "rotation-deadzone", 0),
        trans_deadzone(b, "translation-deadzone", 0)
    {}
};

class FTNoIR_Filter : public IFilter
{
public:
    FTNoIR_Filter();
    void filter(const double* input, double *output);
    Map rot, trans;
private:
    settings_accela s;
    bool first_run;
    double last_output[6];
    double smoothed_input[6];
    Timer t;
};
