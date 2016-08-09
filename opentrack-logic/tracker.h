/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <vector>

#include "opentrack-compat/pi-constant.hpp"
#include "opentrack-compat/timer.hpp"
#include "opentrack/plugin-support.hpp"
#include "mappings.hpp"
#include "simple-mat.hpp"
#include "selected-libraries.hpp"

#include "spline-widget/functionconfig.h"
#include "main-settings.hpp"
#include "opentrack-compat/options.hpp"
#include "tracklogger.hpp"

#include <QMutex>
#include <QThread>

#include "export.hpp"

using Pose = Mat<double, 6, 1>;

class OPENTRACK_LOGIC_EXPORT Tracker : private QThread
{
    Q_OBJECT
private:
    QMutex mtx;
    main_settings s;
    Mappings& m;

    Timer t;
    Pose output_pose, raw_6dof, last_mapped, last_raw;

    double newpose[6];
    volatile bool centerp;
    volatile bool enabledp;
    volatile bool zero_;
    volatile bool should_quit;
    SelectedLibraries const& libs;
    // The owner of the reference is the main window.
    // This design might be usefull if we decide later on to swap out
    // the logger while the tracker is running.
    TrackLogger &logger;

    using rmat = euler::rmat;
    using euler_t = euler::euler_t;

    rmat r_b, r_b_real;
    double t_b[3];

    double map(double pos, Mapping& axis);
    void logic();
    void t_compensate(const rmat& rmat, const euler_t& ypr, euler_t& output, bool rz);
    void run() override;

    static constexpr double pi = OPENTRACK_PI;
    static constexpr double r2d = 180. / OPENTRACK_PI;
    static constexpr double d2r = OPENTRACK_PI / 180.;

    // note: float exponent base is 2
    static constexpr double c_mult = 4;
    static constexpr double c_div = 1./c_mult;
public:
    Tracker(Mappings& m, SelectedLibraries& libs, TrackLogger &logger);
    ~Tracker();

    rmat get_camera_offset_matrix(double c);
    void get_raw_and_mapped_poses(double* mapped, double* raw) const;
    void start() { QThread::start(); }
    void toggle_enabled() { enabledp = !enabledp; }
    void set_toggle(bool value) { enabledp = value; }
    void set_zero(bool value) { zero_ = value; }
    void center() { centerp = !centerp; }
    void zero() { zero_ = !zero_; }
};
