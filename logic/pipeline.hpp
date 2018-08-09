#pragma once

#include <vector>

#include "compat/timer.hpp"
#include "api/plugin-support.hpp"
#include "mappings.hpp"
#include "compat/euler.hpp"
#include "compat/enum-operators.hpp"
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

static constexpr inline double scale_c = 8;
static constexpr inline double scale_inv_c = 1/scale_c;

struct state_
{
    rmat inv_rot_center { rmat::eye() };
    rmat rotation { rmat::eye() };
    euler_t t_center;
};

class reltrans
{
    euler_t interp_pos;
    Timer interp_timer;

    // this implements smooth transition into reltrans mode
    // once not aiming anymore. see `apply_pipeline()'.
    Timer interp_phase_timer;
    unsigned RC_phase = 0;

    bool cur = false;
    bool in_zone = false;

public:
    reltrans();

    void on_center();

    cc_warn_unused_result
    euler_t rotate(const rmat& rmat, const euler_t& in, vec3_bool disable) const;

    cc_warn_unused_result
    Pose apply_pipeline(reltrans_state state, const Pose& value,
                        const vec6_bool& disable, bool neck_enable, int neck_z);

    cc_warn_unused_result
    euler_t apply_neck(const Pose& value, int nz, bool disable_tz) const;
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

    void set(flags flag, bool val);
    void negate(flags flag);
    bool get(flags flag);
    bits();
};

DEFINE_ENUM_OPERATORS(bits::flags);

class OTR_LOGIC_EXPORT pipeline : private QThread, private bits
{
    Q_OBJECT

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

    reltrans rel;

    //state_ state, scaled_state;
    state_ scaled_state;

    ms backlog_time {};

    bool tracking_started = false;

    double map(double pos, Map& axis);
    void logic();
    void run() override;
    bool maybe_enable_center_on_tracking_started();
    void maybe_set_center_pose(const Pose& value, bool own_center_logic);
    void store_tracker_pose(const Pose& value);
    Pose clamp_value(Pose value) const;
    Pose apply_center(Pose value) const;
    std::tuple<Pose, Pose, vec6_bool> get_selected_axis_values(const Pose& newpose) const;
    Pose maybe_apply_filter(const Pose& value) const;
    Pose apply_reltrans(Pose value, vec6_bool disabled, bool centerp);
    Pose apply_zero_pos(Pose value) const;

public:
    pipeline(Mappings& m, runtime_libraries& libs, event_handler& ev, TrackLogger& logger);
    ~pipeline() override;

    void raw_and_mapped_pose(double* mapped, double* raw) const;
    void start() { QThread::start(QThread::HighPriority); }

    void toggle_zero();
    void toggle_enabled();

    void set_center();
    void set_held_center(bool value);
    void set_enabled(bool value);
    void set_zero(bool value);
};

} // ns pipeline_impl

using pipeline = pipeline_impl::pipeline;
