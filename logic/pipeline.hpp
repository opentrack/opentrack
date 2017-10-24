/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <vector>

#include "compat/timer.hpp"
#include "api/plugin-support.hpp"
#include "mappings.hpp"
#include "compat/euler.hpp"
#include "runtime-libraries.hpp"
#include "extensions.hpp"

#include "spline/spline.hpp"
#include "main-settings.hpp"
#include "options/options.hpp"
#include "tracklogger.hpp"

#include <QMutex>
#include <QThread>

#include <atomic>
#include <cmath>

#include "export.hpp"

namespace gui_tracker_impl {

using namespace time_units;

struct OTR_LOGIC_EXPORT bits
{
    enum flags : unsigned {
        f_center         = 1 << 0,
        f_enabled_h      = 1 << 1,
        f_enabled_p      = 1 << 2,
        f_zero           = 1 << 3,
    };

    std::atomic<unsigned> b;

    void set(flags flag_, bool val_);
    void negate(flags flag_);
    bool get(flags flag);
    bits();
};

class OTR_LOGIC_EXPORT pipeline : private QThread, private bits
{
    Q_OBJECT
private:
    using rmat = euler::rmat;
    using euler_t = euler::euler_t;

    QMutex mtx;
    main_settings s;
    Mappings& m;
    event_handler& ev;

    Timer t;
    Pose output_pose, raw_6dof, last_mapped, last_raw;

    Pose newpose;
    runtime_libraries const& libs;
    // The owner of the reference is the main window.
    // This design might be usefull if we decide later on to swap out
    // the logger while the tracker is running.
    TrackLogger& logger;

    struct state
    {
        rmat rot_center;
        rmat rotation;

        state() : rot_center(rmat::eye())
        {}
    };

    state real_rotation, scaled_rotation;
    euler_t t_center;

    ns backlog_time = ns(0);

    bool tracking_started = false;

    double map(double pos, Map& axis);
    void logic();
    void t_compensate(const rmat& rmat, const euler_t& ypr, euler_t& output,
                      bool disable_tx, bool disable_ty, bool disable_tz);
    void run() override;

    static constexpr double r2d = 180. / M_PI;
    static constexpr double d2r = M_PI / 180.;

    // note: float exponent base is 2
    static constexpr double c_mult = 16;
    static constexpr double c_div = 1./c_mult;
public:
    pipeline(Mappings& m, runtime_libraries& libs, event_handler& ev, TrackLogger& logger);
    ~pipeline();

    void raw_and_mapped_pose(double* mapped, double* raw) const;
    void start() { QThread::start(QThread::HighPriority); }

    void center();
    void set_toggle(bool value);
    void set_zero(bool value);
    void zero();
    void toggle_enabled();
};

} // ns impl

using gui_tracker_impl::pipeline;
