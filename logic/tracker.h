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
#include "selected-libraries.hpp"

#include "spline-widget/spline.hpp"
#include "main-settings.hpp"
#include "options/options.hpp"
#include "tracklogger.hpp"

#include <QMutex>
#include <QThread>

#include <atomic>
#include <cmath>

#include "export.hpp"

using Pose = Mat<double, 6, 1>;

struct bits
{
    enum flags {
        f_center         = 1 << 0,
        f_enabled        = 1 << 1,
        f_zero           = 1 << 2,
        f_tcomp_disabled = 1 << 3,
        f_should_quit    = 1 << 4,
    };

    std::atomic<unsigned> b;

    void set(flags flag_, bool val_);
    void negate(flags flag_);
    bool get(flags flag);
    bits();
};

class OPENTRACK_LOGIC_EXPORT Tracker : private QThread, private bits
{
    Q_OBJECT
private:
    using rmat = euler::rmat;
    using euler_t = euler::euler_t;

    QMutex mtx;
    main_settings s;
    Mappings& m;

    Timer t;
    Pose output_pose, raw_6dof, last_mapped, last_raw;

    Pose newpose;
    SelectedLibraries const& libs;
    // The owner of the reference is the main window.
    // This design might be usefull if we decide later on to swap out
    // the logger while the tracker is running.
    TrackLogger& logger;

    struct state
    {
        rmat center_yaw, center_pitch, center_roll;
        rmat rot_center;
        rmat camera;
        rmat rotation, ry, rp, rr;

        state() : center_yaw(rmat::eye()), center_pitch(rmat::eye()), center_roll(rmat::eye()), rot_center(rmat::eye())
        {}
    };

    state real_rotation, scaled_rotation;
    euler_t t_center;

    long backlog_time;

    double map(double pos, Map& axis);
    void logic();
    void t_compensate(const rmat& rmat, const euler_t& ypr, euler_t& output,
                      bool disable_tx, bool disable_ty, bool disable_tz);
    void run() override;

    static constexpr double r2d = 180. / M_PI;
    static constexpr double d2r = M_PI / 180.;

    // note: float exponent base is 2
    static constexpr double c_mult = 4;
    static constexpr double c_div = 1./c_mult;
public:
    Tracker(Mappings& m, SelectedLibraries& libs, TrackLogger& logger);
    ~Tracker();

    rmat get_camera_offset_matrix(double c);
    void get_raw_and_mapped_poses(double* mapped, double* raw) const;
    void start() { QThread::start(); }

    void center() { set(f_center, true); }

    void set_toggle(bool value) { set(f_enabled, value); }
    void set_zero(bool value) { set(f_zero, value); }
    void set_tcomp_disabled(bool x) { set(f_tcomp_disabled, x); }

    void zero() { negate(f_zero); }
    void toggle_enabled() { negate(f_enabled); }
};
