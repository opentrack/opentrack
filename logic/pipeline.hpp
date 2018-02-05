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

using rmat = euler::rmat;
using euler_t = euler::euler_t;

using vec6_bool = Mat<bool, 6, 1>;
using vec3_bool = Mat<bool, 6, 1>;

class reltrans
{
    euler_t interp_pos, last_value;
    Timer interp_timer;
    bool cur = false;
    bool in_zone = false;

public:
    reltrans();

    warn_result_unused
    euler_t rotate(const rmat& rmat, const euler_t& in, vec3_bool disable) const;

    warn_result_unused
    Pose apply_pipeline(reltrans_state cur, const Pose& value, const vec6_bool& disable);

    warn_result_unused
    euler_t apply_neck(const Pose& value, bool enable, int nz) const;
};

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

    reltrans rel;

    state real_rotation, scaled_rotation;
    euler_t t_center;

    ns backlog_time = ns(0);

    bool tracking_started = false;

    double map(double pos, Map& axis);
    void logic();
    void run() override;
    void maybe_enable_center_on_tracking_started();
    void maybe_set_center_pose(const Pose& value, bool own_center_logic);
    Pose clamp_value(Pose value) const;
    Pose apply_center(Pose value) const;
    std::tuple<Pose, Pose, vec6_bool> get_selected_axis_value(const Pose& newpose) const;
    Pose maybe_apply_filter(const Pose& value) const;
    Pose apply_reltrans(Pose value, vec6_bool disabled);
    Pose apply_zero_pos(Pose value) const;

    // note: float exponent base is 2
    static constexpr inline double c_mult = 16;
    static constexpr inline double c_div = 1./c_mult;
public:
    pipeline(Mappings& m, runtime_libraries& libs, event_handler& ev, TrackLogger& logger);
    ~pipeline();

    void raw_and_mapped_pose(double* mapped, double* raw) const;
    void start() { QThread::start(QThread::HighPriority); }

    void toggle_zero();
    void toggle_enabled();

    void set_center();
    void set_enabled(bool value);
    void set_zero(bool value);
};

} // ns impl

using gui_tracker_impl::pipeline;
