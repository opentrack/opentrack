/* Copyright (c) 2012-2018 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

/*
 * this file appeared originally in facetracknoir, was rewritten completely
 * following opentrack fork.
 *
 * originally written by Wim Vriend.
 */

#include "compat/sleep.hpp"
#include "compat/math.hpp"
#include "compat/meta.hpp"
#include "compat/macros.hpp"

#include "pipeline.hpp"
#include "logic/shortcuts.h"

#include <cmath>
#include <algorithm>
#include <cstdio>

#ifdef _WIN32
#   include <windows.h>
#endif

using namespace euler;
using namespace time_units;
using namespace pipeline_impl;

static constexpr inline double r2d = 180. / M_PI;
static constexpr inline double d2r = M_PI / 180.;

reltrans::reltrans() {}

void reltrans::on_center()
{
    interp_pos = { 0, 0, 0 };
    in_zone = false;
    cur = false;
}

euler_t reltrans::rotate(const rmat& R, const euler_t& in, vec3_bool disable) const
{
    enum { tb_Z, tb_X, tb_Y };

    // TY is really yaw axis. need swapping accordingly.
    // sign changes are due to right-vs-left handedness of coordinate system used
    const euler_t ret = R * euler_t(in(TZ), -in(TX), -in(TY));

    euler_t output;

    if (disable(TZ))
        output(TZ) = in(TZ);
    else
        output(TZ) = ret(tb_Z);

    if (disable(TY))
        output(TY) = in(TY);
    else
        output(TY) = -ret(tb_Y);

    if (disable(TX))
        output(TX) = in(TX);
    else
        output(TX) = -ret(tb_X);

    return output;
}

Pose reltrans::apply_pipeline(reltrans_state state, const Pose& value,
                              const vec6_bool& disable, bool neck_enable, int neck_z)
{
    euler_t rel((const double*)(value));

    if (state != reltrans_disabled)
    {
        {
            bool tcomp_in_zone_ = progn(
                if (state == reltrans_non_center)
                {
                    const bool looking_down = value(Pitch) < 20;
                    return looking_down ? std::fabs(value(Yaw)) > 35 : std::fabs(value(Yaw)) > 65;
                }
                else
                    return true;
            );

            if (!cur && in_zone != tcomp_in_zone_)
            {
                //qDebug() << "reltrans-interp: START" << tcomp_in_zone_;
                cur = true;
                interp_timer.start();
                interp_phase_timer.start();
                RC_phase = 0;
            }

            in_zone = tcomp_in_zone_;
        }

        // only when looking behind or downward
        if (in_zone)
        {
            const rmat R = euler_to_rmat(euler_t(value(Yaw)   * d2r * !disable(Yaw),
                                                 value(Pitch) * d2r * !disable(Pitch),
                                                 value(Roll)  * d2r * !disable(Roll)));

            rel = rotate(R, rel, &disable[TX]);

            // dynamic neck
            if (neck_enable)
            {
                const euler_t neck = apply_neck(value, -neck_z);

                for (unsigned k = 0; k < 3; k++)
                    rel(k) += neck(k);
            }
        }

        if (cur)
        {
            const double dt = interp_timer.elapsed_seconds();

            static constexpr float RC_phases[] = { 2, 1, .5, .1, .025 };
            static constexpr float RC_time_deltas[] = { 1, .25, .25, 2 };

            interp_timer.start();

            if (RC_phase + 1 != std::size(RC_phases) &&
                interp_phase_timer.elapsed_seconds() > RC_time_deltas[RC_phase])
            {
                RC_phase++;
                interp_phase_timer.start();
            }

            const double RC = RC_phases[RC_phase];
            const double alpha = dt/(dt+RC);

            constexpr double eps = .01;

            interp_pos = interp_pos * (1-alpha) + rel * alpha;

            const euler_t tmp = rel - interp_pos;
            rel = interp_pos;
            const double delta = std::fabs(tmp(0)) + std::fabs(tmp(1)) + std::fabs(tmp(2));

            //qDebug() << "reltrans-interp: delta" << delta << "phase" << RC_phase;

            if (delta < eps)
            {
                //qDebug() << "reltrans-interp: STOP";
                cur = false;
            }
        }
        else
        {
            interp_pos = rel;
        }
    }
    else
    {
        cur = false;
        in_zone = false;

        // dynamic neck
        if (neck_enable)
        {
            const euler_t neck = apply_neck(value, -neck_z);

            for (unsigned k = 0; k < 3; k++)
                rel(k) += neck(k);
        }
    }

    return {
        rel(TX), rel(TY), rel(TZ),
        value(Yaw), value(Pitch), value(Roll),
    };
}

euler_t reltrans::apply_neck(const Pose& value, int nz) const
{
    euler_t neck;

    const rmat R = euler_to_rmat(euler_t(&value[Yaw]) * d2r);
    neck = rotate(R, { 0, 0, nz }, vec3_bool());
    neck(TZ) = neck(TZ) - nz;

    return neck;
}

pipeline::pipeline(Mappings& m, runtime_libraries& libs, event_handler& ev, TrackLogger& logger) :
    m(m),
    ev(ev),
    libs(libs),
    logger(logger)
{
}

pipeline::~pipeline()
{
    requestInterruption();
    wait();
}

double pipeline::map(double pos, Map& axis)
{
    bool altp = (pos < 0) && axis.opts.altp;
    axis.spline_main.set_tracking_active( !altp );
    axis.spline_alt.set_tracking_active( altp );
    auto& fc = altp ? axis.spline_alt : axis.spline_main;
    return double(fc.get_value(pos));
}

template<int u, int w>
static bool is_nan(const dmat<u,w>& r)
{
    for (int i = 0; i < u; i++)
        for (int j = 0; j < w; j++)
        {
            int val = std::fpclassify(r(i, j));
            if (val == FP_NAN || val == FP_INFINITE)
                return true;
        }

    return false;
}

template<typename x>
static force_inline bool nan_check_(const x& datum)
{
    return is_nan(datum);
}

template<typename x, typename y, typename... xs>
static force_inline bool nan_check_(const x& datum, const y& next, const xs&... rest)
{
    return is_nan(datum) || nan_check_(next, rest...);
}

static never_inline
void emit_nan_check_msg(const char* text, const char* fun, int line)
{
    once_only(
        qDebug()  << "nan check failed"
                  << "for:" << text
                  << "function:" << fun
                  << "line:" << line;
    );
}

template<typename... xs>
static never_inline
bool maybe_nan(const char* text, const char* fun, int line, const xs&... vals)
{
    if (nan_check_(vals...))
    {
        emit_nan_check_msg(text, fun, line);
        return true;
    }
    return false;
}

// for MSVC `else' is like `unlikely' for GNU

#define nan_check(...)                                                              \
    do                                                                              \
    {                                                                               \
        if (likely(!maybe_nan(#__VA_ARGS__, OTR_FUNNAME, __LINE__, __VA_ARGS__)))   \
            (void)0;                                                                \
        else                                                                        \
            goto error;                                                             \
    } while (false)

bool pipeline::maybe_enable_center_on_tracking_started()
{
    if (!tracking_started)
    {
        for (int i = 0; i < 6; i++)
            if (std::fabs(newpose(i)) != 0)
            {
                tracking_started = true;
                break;
            }

        if (tracking_started && s.center_at_startup)
        {
            set(f_center, true);
            return true;
        }
    }

    return false;
}

void pipeline::maybe_set_center_pose(const Pose& value, bool own_center_logic)
{
    euler_t tmp = d2r * euler_t(&value[Yaw]);
    //scaled_rotation.rotation = euler_to_rmat(c_div * tmp);
    rotation.rotation = euler_to_rmat(tmp);

    if (get(f_center | f_held_center))
    {
        if (libs.pFilter)
            libs.pFilter->center();

        if (own_center_logic)
        {
            //scaled_rotation.rot_center = rmat::eye();
            rotation.inv_rot_center = rmat::eye();

            t_center = euler_t();
        }
        else
        {
            rotation.inv_rot_center = rotation.rotation.t();
            //scaled_rotation.rot_center = scaled_rotation.rotation.t();

            t_center = euler_t((const double*)(value));
        }
    }
}

Pose pipeline::clamp_value(Pose value) const
{
    // hatire, udp, and freepie trackers can mess up here
    for (unsigned i = 3; i < 6; i++)
    {
        using std::fmod;
        using std::copysign;
        using std::fabs;

        value(i) = fmod(value(i), 360);

        const double x = value(i);
        if (fabs(x) - 1e-2 > 180)
            value(i) = fmod(x + copysign(180, x), 360) - copysign(180, x);
        else
            value(i) = clamp(x, -180, 180);
    }

    return value;
}

Pose pipeline::apply_center(Pose value) const
{
    euler_t T = euler_t(value) - t_center;
    euler_t R = r2d * rmat_to_euler(rotation.rotation * rotation.inv_rot_center);

    T = rel.rotate(rotation.inv_rot_center, T, vec3_bool());

    for (int i = 0; i < 3; i++)
    {
        // don't invert after reltrans
        // inverting here doesn't break centering

        if (m(i+3).opts.invert)
            R(i) = -R(i);
        if (m(i).opts.invert)
            T(i) = -T(i);
    }

    for (int i = 0; i < 3; i++)
    {
        value(i) = T(i);
        value(i+3) = R(i);
    }

    return value;
}

std::tuple<Pose, Pose, vec6_bool>
pipeline::get_selected_axis_value(const Pose& newpose) const
{
    Pose value;
    vec6_bool disabled;

    for (int i = 0; i < 6; i++)
    {
        const Map& axis = m(i);
        const int k = axis.opts.src;

        disabled(i) = k == 6;

        if (k < 0 || k >= 6)
            value(i) = 0;
        else
            value(i) = newpose(k);
    }

    return { newpose, value, disabled };
}

Pose pipeline::maybe_apply_filter(const Pose& value) const
{
    Pose tmp(value);

    // nan/inf values will corrupt filter internal state
    if (libs.pFilter)
        libs.pFilter->filter(value, tmp);

    return tmp;
}

Pose pipeline::apply_zero_pos(Pose value) const
{
    // custom zero position
    for (int i = 0; i < 6; i++)
        value(i) += m(i).opts.zero * (m(i).opts.invert ? -1 : 1);

    return value;
}

Pose pipeline::apply_reltrans(Pose value, vec6_bool disabled, bool centerp)
{
    if (centerp)
        rel.on_center();

    value = rel.apply_pipeline(s.reltrans_mode, value,
                               { !!s.reltrans_disable_src_yaw,
                                 !!s.reltrans_disable_src_pitch,
                                 !!s.reltrans_disable_src_roll,
                                 !!s.reltrans_disable_tx,
                                 !!s.reltrans_disable_ty,
                                 !!s.reltrans_disable_tz },
                               s.neck_enable,
                               s.neck_z);

    // reltrans will move it
    for (unsigned k = 0; k < 6; k++)
        if (disabled(k))
            value(k) = 0;

    return value;
}

void pipeline::logic()
{
    using namespace euler;
    using EV = event_handler::event_ordinal;

    logger.write_dt();
    logger.reset_dt();

    // we must center prior to getting data from the tracker
    const bool center_ordered = get(f_center | f_held_center) && tracking_started;
    const bool own_center_logic = center_ordered && libs.pTracker->center();
    const bool hold_ordered = get(f_enabled_p) ^ get(f_enabled_h);

    {
        Pose tmp;
        libs.pTracker->data(tmp);
        ev.run_events(EV::ev_raw, tmp);
        newpose = tmp;
    }

    auto [raw, value, disabled] = get_selected_axis_value(newpose);
    logger.write_pose(raw); // raw

    nan_check(newpose, raw, value);

    value = clamp_value(value);

    {
        maybe_enable_center_on_tracking_started();
        maybe_set_center_pose(value, own_center_logic);
        value = apply_center(value);
        // "corrected" - after various transformations to account for camera position
        logger.write_pose(value);
    }

    {
        ev.run_events(EV::ev_before_filter, value);
        Pose tmp = maybe_apply_filter(value);
        nan_check(tmp);
        if (!center_ordered)
            value = tmp;
        logger.write_pose(value); // "filtered"
    }

    {
        ev.run_events(EV::ev_before_mapping, value);
        // CAVEAT rotation only, due to tcomp
        for (int i = 3; i < 6; i++)
            value(i) = map(value(i), m(i));
    }

    value = apply_reltrans(value, disabled, center_ordered);

    {
        // CAVEAT translation only, due to tcomp
        for (int i = 0; i < 3; i++)
            value(i) = map(value(i), m(i));
        nan_check(value);
    }

    // we must proceed with all the filtering since the filter
    // needs fresh values to prevent deconvergence
    // same for other stuff that needs dt, it could go stupid

    if (!hold_ordered)
        goto ok;

error:
    {
        QMutexLocker foo(&mtx);

        value = output_pose;
        raw = raw_6dof;

        // for widget last value display
        for (int i = 0; i < 6; i++)
            (void)map(raw_6dof(i), m(i));
    }

ok:

    set(f_center, false);

    if (get(f_zero))
        for (int i = 0; i < 6; i++)
            value(i) = 0;

    value = apply_zero_pos(value);

    ev.run_events(EV::ev_finished, value);
    libs.pProtocol->pose(value);

    QMutexLocker foo(&mtx);
    output_pose = value;
    raw_6dof = raw;

    logger.write_pose(value); // "mapped"

    logger.reset_dt();
    logger.next_line();
}

void pipeline::run()
{
#if defined _WIN32
    const MMRESULT mmres = timeBeginPeriod(1);
#endif

    {
        static const char* const posechannels[6] = { "TX", "TY", "TZ", "Yaw", "Pitch", "Roll" };
        static const char* const datachannels[5] = { "dt", "raw", "corrected", "filtered", "mapped" };

        logger.write(datachannels[0]);
        char buffer[128];
        for (unsigned j = 1; j < 5; ++j)
        {
            for (unsigned i = 0; i < 6; ++i)
            {
                std::sprintf(buffer, "%s%s", datachannels[j], posechannels[i]);
                logger.write(buffer);
            }
        }
        logger.next_line();
    }

    logger.reset_dt();

    t.start();

    while (!isInterruptionRequested())
    {
        logic();

        constexpr ns const_sleep_ms(time_cast<ns>(ms(4)));
        const ns elapsed_nsecs = prog1(t.elapsed<ns>(), t.start());

        if (backlog_time > secs_(3) || backlog_time < secs_(-3))
        {
            qDebug() << "tracker: backlog interval overflow"
                     << time_cast<ms>(backlog_time).count() << "ms";
            backlog_time = backlog_time.zero();
        }

        backlog_time += ns(elapsed_nsecs - const_sleep_ms);

        const int sleep_time_ms = time_cast<ms>(clamp(const_sleep_ms - backlog_time,
                                                      ms::zero(), ms(10))).count();

#if 0
        qDebug() << "sleepy time" << sleep_time_ms
                 << "elapsed" << time_cast<ms>(elapsed_nsecs).count()
                 << "backlog" << time_cast<ms>(backlog_time).count();
#endif

        portable::sleep(sleep_time_ms);
    }

    // filter may inhibit exact origin
    Pose p;
    libs.pProtocol->pose(p);

    for (int i = 0; i < 6; i++)
    {
        m(i).spline_main.set_tracking_active(false);
        m(i).spline_alt.set_tracking_active(false);
    }

#if defined _WIN32
    if (mmres == 0)
        (void) timeEndPeriod(1);
#endif
}

void pipeline::raw_and_mapped_pose(double* mapped, double* raw) const
{
    QMutexLocker foo(&const_cast<pipeline&>(*this).mtx);

    for (int i = 0; i < 6; i++)
    {
        raw[i] = raw_6dof(i);
        mapped[i] = output_pose(i);
    }
}

void pipeline::set_center() { set(f_center, true); }

void pipeline::set_held_center(bool value)
{
    set(f_held_center, value);
}

void pipeline::set_enabled(bool value) { set(f_enabled_h, value); }
void pipeline::set_zero(bool value) { set(f_zero, value); }

void pipeline::toggle_zero() { negate(f_zero); }
void pipeline::toggle_enabled() { negate(f_enabled_p); }

void bits::set(flags flag_, bool val_)
{
    const unsigned flag = unsigned(flag_);
    const unsigned val = unsigned(val_);

    for (;;)
    {
        unsigned b_(b);
        if (b.compare_exchange_weak(b_,
                                    unsigned((b_ & ~flag) | (flag * val)),
                                    std::memory_order_seq_cst,
                                    std::memory_order_seq_cst))
            break;
    }
}

void bits::negate(flags flag_)
{
    const unsigned flag = unsigned(flag_);

    for (;;)
    {
        unsigned b_(b);

        if (b.compare_exchange_weak(b_,
                                    b_ ^ flag,
                                    std::memory_order_seq_cst,
                                    std::memory_order_seq_cst))
            break;
    }
}

bool bits::get(flags flag)
{
    return !!(b & flag);
}

bits::bits() : b(0u)
{
    set(f_center, false);
    set(f_held_center, false);
    set(f_enabled_p, true);
    set(f_enabled_h, true);
    set(f_zero, false);
}
