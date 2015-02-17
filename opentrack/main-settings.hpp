#pragma once

#include <QString>
#include "opentrack/options.hpp"
#include "opentrack/plugin-api.hpp"

using namespace options;

struct axis_opts {
    value<double> zero;
    value<bool> invert, altp;
    value<int> src;
    axis_opts(pbundle b, QString pfx, int idx) :
        zero(b, n(pfx, "zero-pos"), 0),
        invert(b, n(pfx, "invert-sign"), false),
        altp(b, n(pfx, "alt-axis-sign"), false),
        src(b, n(pfx, "source-index"), idx)
    {}
private:
    static inline QString n(QString pfx, QString name) {
        return QString("%1-%2").arg(pfx, name);
    }
};

struct main_settings {
    pbundle b;
    value<QString> tracker_dll, tracker2_dll, filter_dll, protocol_dll;
    axis_opts a_x, a_y, a_z, a_yaw, a_pitch, a_roll;
    value<bool> tcomp_p, tcomp_tz;
    value<bool> tray_enabled;
    value<int> camera_yaw, camera_pitch;
    value<bool> center_at_startup;
    main_settings(pbundle b) :
        b(b),
        tracker_dll(b, "tracker-dll", ""),
        tracker2_dll(b, "tracker2-dll", ""),
        filter_dll(b, "filter-dll", ""),
        protocol_dll(b, "protocol-dll", ""),
        a_x(b, "x", TX),
        a_y(b, "y", TY),
        a_z(b, "z", TZ),
        a_yaw(b, "yaw", Yaw),
        a_pitch(b, "pitch", Pitch),
        a_roll(b, "roll", Roll),
        tcomp_p(b, "compensate-translation", true),
        tcomp_tz(b, "compensate-translation-disable-z-axis", false),
        tray_enabled(b, "use-system-tray", false),
        camera_yaw(b, "camera-yaw", 0),
        camera_pitch(b, "camera-pitch", 0),
        center_at_startup(b, "center-at-startup", true)
    {}
};
