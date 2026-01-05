#include "axis-opts.hpp"

namespace axis_opts_impl {

using max_clamp = axis_opts::max_clamp;

static max_clamp get_max_x(Axis k)
{
    if (k == Axis(-1))
        return max_clamp::x1000;
    if (k == Pitch)
        return max_clamp::r90;
    if (k >= Yaw)
        return max_clamp::r180;
    return max_clamp::t30;
}

static max_clamp get_max_y(Axis k)
{
    if (k == Axis(-1))
        return max_clamp::x1000;
    if (k == Axis::Pitch)
        return max_clamp::o_r90;
    if (k >= Axis::Yaw)
        return max_clamp::o_r180;
    return max_clamp::o_t75;
}

axis_opts::axis_opts(QString pfx, Axis idx) :
    prefix_(pfx),
    axis_(idx),
    zero(b_settings_window, n(pfx, "zero-pos"), 0),
    src(b_settings_window, n(pfx, "source-index"), idx),
    invert_pre(b_settings_window, n(pfx, "invert-sign"), false),
    invert_post(b_settings_window, n(pfx, "invert-sign-post"), false),
    altp(b_mapping_window, n(pfx, "alt-axis-sign"), false),
    clamp_x_(b_mapping_window, n(pfx, "max-value"), get_max_x(idx)),
    clamp_y_(b_mapping_window, n(pfx, "max-output-value"), get_max_y(idx))
{}

QString const& axis_opts::prefix() const { return prefix_; }

Axis axis_opts::axis() const { return axis_; }

QString axis_opts::n(QString const& pfx, QString const& name)
{
    return QString("%1-%2").arg(pfx, name);
}

} // ns axis_opts_impl
