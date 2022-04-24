#pragma once

#include "options/options.hpp"
#include "api/plugin-api.hpp"

#include "export.hpp"

namespace axis_opts_impl {

using namespace options;

class OTR_SPLINE_EXPORT axis_opts final
{
    QString prefix_;
    Axis axis_;

    static inline QString n(QString const& pfx, QString const& name);

public:
    enum max_clamp
    {
        r180 = 180,
        r90 = 90,
        r60 = 60,
        r45 = 45,
        r30 = 30,
        r25 = 25,
        r20 = 20,
        r15 = 15,
        r10 = 10,

        t600 = 600,
        t300 = 300,
        t150 = 150,
        t100 = 100,
        t75 = 75,
        t30 = 30,
        t20 = 20,
        t15 = 15,
        t10 = 10,

        o_r180 = -180,
        o_r90 = -90,
        o_t75 = -75,
        o_t100 = -100,
        o_t150 = -150,
        o_t300 = -300,
        o_t600 = -600,

        x1000 = 1000,
    };

    // note, these two bundles can be the same value with no issues
    bundle b_settings_window{ make_bundle(axis_ == Axis(-1) ? QString() : "opentrack-ui") };
    bundle b_mapping_window{ make_bundle(axis_ == Axis(-1) ? QString() : "opentrack-mappings") };
    value<double> zero;
    value<int> src;
    value<bool> invert, altp;
    value<max_clamp> clamp_x_, clamp_y_;
    double max_clamp_x() const { return std::fabs(clamp_x_.to<double>()); }
    double max_clamp_y() const { return std::fabs(clamp_y_.to<double>()); }
    axis_opts(QString pfx, Axis idx);

    QString const& prefix() const;
    Axis axis() const;
};

} // ns axis_opts_impl

using axis_opts = axis_opts_impl::axis_opts;
