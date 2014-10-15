#pragma once

#include <QString>
#include "facetracknoir/options.h"
using namespace options;

struct key_opts {
    value<int> key_index;
    value<bool> ctrl, alt, shift;
    key_opts(pbundle b, const QString& name) :
        key_index(b,  QString("key-index-%1").arg(name), 0),
        ctrl(b,  QString("key-ctrl-%1").arg(name), 0),
        alt(b,  QString("key-alt-%1").arg(name), 0),
        shift(b,  QString("key-shift-%1").arg(name), 0)
    {}
};

struct axis_opts {
    value<double> zero;
    value<bool> invert, altp;
    value<int> src;
    axis_opts(pbundle b, QString pfx, int idx) :
        zero(b, n(pfx, "zero-pos"), 0),
        invert(b, n(pfx, "invert-axis"), false),
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
    key_opts center_key;
    key_opts toggle_key;
    value<QString> tracker_dll, tracker2_dll, filter_dll, protocol_dll;
    axis_opts a_x, a_y, a_z, a_yaw, a_pitch, a_roll;
    value<bool> tcomp_p, tcomp_tz, dingp;
    main_settings(pbundle b) :
        b(b),
        center_key(b, "center"),
        toggle_key(b, "toggle"),
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
        dingp(b, "ding", true)
    {}
};
