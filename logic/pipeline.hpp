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

namespace pipeline_impl {

using rmat = euler::rmat;
using euler_t = euler::euler_t;

using vec6_bool = Mat<bool, 6, 1>;
using vec3_bool = Mat<bool, 6, 1>;

class reltrans
{
    euler_t interp_pos;
    Timer interp_timer;

    // this implements smooth transition into reltrans mode
    // once not aiming anymore. see `apply_pipeline()'.
    Timer interp_phase_timer;
    unsigned RC_phase;

    bool cur = false;
    bool in_zone = false;

public:
    reltrans();

    void on_center();

    warn_result_unused
    euler_t rotate(const rmat& rmat, const euler_t& in, vec3_bool disable) const;

    warn_result_unused
    Pose apply_pipeline(reltrans_state state, const Pose& value,
                        const vec6_bool& disable, bool neck_enable, int neck_z);

    warn_result_unused
    euler_t apply_neck(const Pose& value, int nz) const;
};

using namespace time_units;

struct OTR_LOGIC_EXPORT bits
{
    enum flags : unsigned {
        f_center         = 1 << 0,
        f_held_center    = 1 << 1,
        f_enabled_h      = 1 << 2,
        f_enabled_p      = 1 << 3,
        f_zero           = 1 << 4,
    };

    std::atomic<unsigned> b;

    void set(flags flag_, bool val_);
    void negate(flags flag_);
    bool get(unsigned flag);
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
    // This design might be useful if we decide later on to swap out
    // the logger while the tracker is running.
    TrackLogger& logger;

    struct state
    {
        rmat inv_rot_center;
        rmat rotation;

        state() : inv_rot_center(rmat::eye())
        {}
    };

    reltrans rel;

    state rotation;
    euler_t t_center;

    ns backlog_time = ns(0);

    bool tracking_started = false;

    double map(double pos, Map& axis);
    void logic();
    void run() override;
    bool maybe_enable_center_on_tracking_started();
    void maybe_set_center_pose(const Pose& value, bool own_center_logic);
    Pose clamp_value(Pose value) const;
    Pose apply_center(Pose value) const;
    std::tuple<Pose, Pose, vec6_bool> get_selected_axis_value(const Pose& newpose) const;
    Pose maybe_apply_filter(const Pose& value) const;
    Pose apply_reltrans(Pose value, vec6_bool disabled, bool centerp);
    Pose apply_zero_pos(Pose value) const;

public:
    pipeline(Mappings& m, runtime_libraries& libs, event_handler& ev, TrackLogger& logger);
    ~pipeline();

    void raw_and_mapped_pose(double* mapped, double* raw) const;
    void start() { QThread::start(QThread::HighPriority); }

    void toggle_zero();
    void toggle_enabled();

    void set_center();
    void set_held_center(bool value);
    void set_enabled(bool value);
    void set_zero(bool value);
};

} // ns pipeine_impl

using pipeline_impl::pipeline;
