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
#   include <mmsystem.h>
#endif

namespace pipeline_impl {

reltrans::reltrans() = default;

void reltrans::on_center()
{
    interp_pos = { 0, 0, 0 };
    in_zone = false;
    moving_to_reltans = false;
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
    euler_t rel((const double*)value);

    if (state != reltrans_disabled)
    {
        bool in_zone_ = true;
        if (state == reltrans_non_center)
        {
            const bool looking_down = value(Pitch) < 20;
            in_zone_ = looking_down ? std::fabs(value(Yaw)) > 35 : std::fabs(value(Yaw)) > 65;
        }

        if (!moving_to_reltans && in_zone != in_zone_)
        {
            //qDebug() << "reltrans-interp: START" << tcomp_in_zone_;
            moving_to_reltans = true;
            interp_timer.start();
            interp_phase_timer.start();
            RC_stage = 0;
        }

        in_zone = in_zone_;

        // only when looking behind or downward
        if (in_zone)
        {
            constexpr double d2r = M_PI / 180;

            const rmat R = euler_to_rmat(
                               euler_t(value(Yaw)   * d2r * !disable(Yaw),
                                       value(Pitch) * d2r * !disable(Pitch),
                                       value(Roll)  * d2r * !disable(Roll)));

            rel = rotate(R, rel, &disable[TX]);

            // dynamic neck
            if (neck_enable && (state != reltrans_non_center || !in_zone))
            {
                const euler_t neck = apply_neck(R, -neck_z, disable(TZ));

                for (unsigned k = 0; k < 3; k++)
                    rel(k) += neck(k);
            }
        }

        if (moving_to_reltans)
        {
            const double dt = interp_timer.elapsed_seconds();

            static constexpr double RC_stages[] = { 2, 1, .5, .1, .05 };
            static constexpr double RC_time_deltas[] = { 1, .25, .25, 2 };

            interp_timer.start();

            if (RC_stage + 1 < std::size(RC_stages) &&
                interp_phase_timer.elapsed_seconds() > RC_time_deltas[RC_stage])
            {
                RC_stage++;
                interp_phase_timer.start();
            }

            const double RC = RC_stages[RC_stage];
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
                moving_to_reltans = false;
            }
        }
        else
            interp_pos = rel;
    }
    else
    {
        moving_to_reltans = false;
        in_zone = false;
    }

    return {
        rel(TX), rel(TY), rel(TZ),
        value(Yaw), value(Pitch), value(Roll),
    };
}

euler_t reltrans::apply_neck(const rmat& R, int nz, bool disable_tz) const
{
    euler_t neck;

    neck = rotate(R, { 0, 0, nz }, {});
    neck(TZ) = neck(TZ) - nz;

    if (disable_tz)
        neck(TZ) = 0;

    return neck;
}

pipeline::pipeline(Mappings& m, runtime_libraries& libs, event_handler& ev, TrackLogger& logger) :
    m(m), ev(ev), libs(libs), logger(logger)
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
    axis.spline_main.set_tracking_active(!altp);
    axis.spline_alt.set_tracking_active(altp);
    auto& fc = altp ? axis.spline_alt : axis.spline_main;
    return double(fc.get_value(pos));
}

//#define NO_NAN_CHECK
//#define DEBUG_TIMINGS

#ifndef NO_NAN_CHECK

template<int u, int w>
static inline bool is_nan(const dmat<u,w>& r)
{
    for (unsigned i = 0; i < u; i++)
        for (unsigned j = 0; j < w; j++)
        {
            int val = std::fpclassify(r(i, j));
            if (val == FP_NAN || val == FP_INFINITE)
                return true;
        }

    return false;
}

static cc_noinline
void emit_nan_check_msg(const char* text, const char* fun, int line)
{
    eval_once(
        qDebug()  << "nan check failed"
                  << "for:" << text
                  << "function:" << fun
                  << "line:" << line
    );
}

template<typename... xs>
static cc_noinline
bool maybe_nan(const char* text, const char* fun, int line, const xs&... vals)
{
    bool ret = (is_nan(vals) || ... || false);

    if (ret)
        emit_nan_check_msg(text, fun, line);

    return ret;
}

#define nan_check(...)                                                                  \
    do                                                                                  \
    {                                                                                   \
        if (likely(!maybe_nan(#__VA_ARGS__, cc_function_name, __LINE__, __VA_ARGS__)))  \
            (void)0;                                                                    \
        else                                                                            \
            goto error;                                                                 \
    }                                                                                   \
    while (false)

#else
#   define nan_check(...) (void)(__VA_ARGS__)
#endif

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
            set_center(true);
            return true;
        }
    }

    return false;
}

void pipeline::maybe_set_center_pose(const Pose& value, bool own_center_logic)
{
    if (b.get(f_center | f_held_center))
    {
        set_center(false);

        if (libs.pFilter)
            libs.pFilter->center();

        if (own_center_logic)
            center = {};
        else
            center = value;
    }
}

Pose pipeline::clamp_value(Pose value) const
{
    // hatire, udp, and freepie trackers can mess up here
    for (unsigned i = 3; i < 6; i++)
    {
        value(i) = std::fmod(value(i), 360);

        const double x = value(i);
        if (std::fabs(x) - 1e-2 > 180)
            value(i) = std::fmod(x + std::copysign(180, x), 360) - std::copysign(180, x);
        value(i) = clamp(x, -180, 180);
    }

    return value;
}

Pose pipeline::apply_center(Pose value) const
{
    // this is incorrect but people like it
    value = value - center;

    for (int i = 0; i < 6; i++)
        // don't invert after reltrans
        // inverting here doesn't break centering
        if (m(i).opts.invert)
            value(i) = -value(i);

    return value;
}

std::tuple<Pose, Pose, vec6_bool>
pipeline::get_selected_axis_values(const Pose& newpose) const
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
    for (int i = 0; i < 6; i++)
        value(i) += m(i).opts.zero * (m(i).opts.invert ? -1 : 1);

    return value;
}

Pose pipeline::apply_reltrans(Pose value, vec6_bool disabled, bool centerp)
{
    if (centerp)
        rel.on_center();

    value = rel.apply_pipeline(s.reltrans_mode, value,
                               { !!s.reltrans_disable_tx,
                                 !!s.reltrans_disable_ty,
                                 !!s.reltrans_disable_tz,
                                 !!s.reltrans_disable_src_yaw,
                                 !!s.reltrans_disable_src_pitch,
                                 !!s.reltrans_disable_src_roll, },
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
    const bool center_ordered = b.get(f_center | f_held_center) && tracking_started;
    const bool own_center_logic = center_ordered && libs.pTracker->center();
    const bool hold_ordered = b.get(f_enabled_p) ^ b.get(f_enabled_h);

    {
        Pose tmp;
        libs.pTracker->data(tmp);
        ev.run_events(EV::ev_raw, tmp);
        newpose = tmp;
    }

    auto [raw, value, disabled] = get_selected_axis_values(newpose);
    logger.write_pose(raw); // raw

    nan_check(newpose, raw, value);

    value = clamp_value(value);

    {
        maybe_enable_center_on_tracking_started();
        maybe_set_center_pose(value, own_center_logic);
        value = clamp_value(apply_center(value));

        // "corrected" - after various transformations to account for camera position
        logger.write_pose(value);
    }

    {
        ev.run_events(EV::ev_before_filter, value);
        // we must proceed with all the filtering since the filter
        // needs fresh values to prevent deconvergence
        if (center_ordered)
            (void)maybe_apply_filter(value);
        else
            value = maybe_apply_filter(value);
        nan_check(value);
        logger.write_pose(value); // "filtered"
    }

    {
        ev.run_events(EV::ev_before_mapping, value);
        // CAVEAT rotation only, due to reltrans
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

    set_center(false);

    if (b.get(f_zero))
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

    setPriority(QThread::HighPriority);
    setPriority(QThread::HighestPriority);

    {
        static const char* const posechannels[6] = { "TX", "TY", "TZ", "Yaw", "Pitch", "Roll" };
        static const char* const datachannels[5] = { "dt", "raw", "corrected", "filtered", "mapped" };

        logger.write(datachannels[0]);
        char buffer[16];
        for (unsigned j = 1; j < 5; ++j) // NOLINT(modernize-loop-convert)
        {
            for (unsigned i = 0; i < 6; ++i) // NOLINT(modernize-loop-convert)
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

        constexpr ns const_sleep_ms(ms{4});
        const ns elapsed_nsecs = t.elapsed<ns>();
        t.start();

        if (std::chrono::abs(backlog_time) > secs(3))
        {
            qDebug() << "tracker: backlog interval overflow"
                     << ms{backlog_time}.count() << "ms";
            backlog_time = {};
        }

        backlog_time += ns{elapsed_nsecs - const_sleep_ms};

        const int sleep_time_ms = (int)(
            clamp(ms{const_sleep_ms - backlog_time},
                  ms{0}, ms{10}).count() - .45f
        );

#ifdef DEBUG_TIMINGS
        {
            static int cnt;
            static Timer tt;
            if (tt.elapsed_seconds() >= 1)
            {
                tt.start();
                qDebug() << cnt << "Hz"
                         << "sleepy time" << sleep_time_ms
                         << "diff" << ms{elapsed_nsecs}.count() - ms{const_sleep_ms}.count()
                         << "backlog" << ms{backlog_time}.count();
                cnt = 0;
            }
            cnt++;
        }
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
    QMutexLocker foo(&mtx);

    for (int i = 0; i < 6; i++)
    {
        raw[i] = raw_6dof(i);
        mapped[i] = output_pose(i);
    }
}

void pipeline::set_center(bool x) { b.set(f_center, x); }

void pipeline::set_held_center(bool value)
{
    b.set(f_held_center, value);
}

void pipeline::set_enabled(bool value) { b.set(f_enabled_h, value); }
void pipeline::set_zero(bool value) { b.set(f_zero, value); }

void pipeline::toggle_zero() { b.negate(f_zero); }
void pipeline::toggle_enabled() { b.negate(f_enabled_p); }

void bits::set(bit_flags flag, bool val)
{
    spinlock_guard l(lock);

    flags &= ~flag;
    if (val)
        flags |= flag;
}

void bits::negate(bit_flags flag)
{
    spinlock_guard l(lock);

    flags ^= flag;
}

bool bits::get(bit_flags flag)
{
    spinlock_guard l(lock);

    return !!(flags & flag);
}

bits::bits()
{
    set(f_center, false);
    set(f_held_center, false);
    set(f_enabled_p, true);
    set(f_enabled_h, true);
    set(f_zero, false);
}

} // ns pipeline_impl
