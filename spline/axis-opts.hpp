#pragma once

#include "options/options.hpp"
using namespace options;
#include "api/plugin-api.hpp"

#include "export.hpp"

struct OTR_SPLINE_EXPORT axis_opts final
{
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

        t100 = 100,
        t30 = 30,
        t20 = 20,
        t15 = 15,
        t10 = 10,

        o_r180 = -180,
        o_r90 = -90,
        o_t75 = -75,

        x1000 = 1000,
    };

    // note, these two bundles can be the same value with no issues
    bundle b_settings_window = make_bundle("opentrack-ui");
    bundle b_mapping_window = make_bundle("opentrack-mappings");
    value<double> zero;
    value<int> src;
    value<bool> invert, altp;
    value<max_clamp> clamp_x_, clamp_y_;
    double max_clamp_x() const { return std::fabs(clamp_x_.to<double>()); }
    double max_clamp_y() const { return std::fabs(clamp_y_.to<double>()); }
    axis_opts(QString pfx, Axis idx);

    QString const& prefix() const;
    Axis axis() const;
private:
    static inline QString n(QString pfx, QString name);

    QString prefix_;
    Axis axis_;
};
