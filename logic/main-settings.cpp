#include "main-settings.hpp"

main_settings::main_settings() :
    b(make_bundle("opentrack-ui")),
    b_map(make_bundle("opentrack-mappings")),
    a_x(b, b_map, "x", TX),
    a_y(b, b_map, "y", TY),
    a_z(b, b_map, "z", TZ),
    a_yaw(b, b_map, "yaw", Yaw),
    a_pitch(b, b_map, "pitch", Pitch),
    a_roll(b, b_map, "roll", Roll),
    tcomp_p(b, "compensate-translation", false),
    tcomp_disable_tx(b, "compensate-translation-disable-x-axis", false),
    tcomp_disable_ty(b, "compensate-translation-disable-y-axis", false),
    tcomp_disable_tz(b, "compensate-translation-disable-z-axis", false),
    tcomp_disable_src_yaw(b, "compensate-translation-disable-source-yaw", false),
    tcomp_disable_src_pitch(b, "compensate-translation-disable-source-pitch", false),
    tcomp_disable_src_roll(b, "compensate-translation-disable-source-roll", false),
    tray_enabled(b, "use-system-tray", false),
    tray_start(b, "start-in-tray", false),
    camera_yaw(b, "camera-yaw", 0),
    camera_pitch(b, "camera-pitch", 0),
    camera_roll(b, "camera-roll", 0),
    center_at_startup(b, "center-at-startup", true),
    center_method(b, "centering-method", 1),
    neck_z(b, "neck-depth", 0),
    neck_enable(b, "neck-enable", false),
    key_start_tracking1(b, "start-tracking"),
    key_start_tracking2(b, "start-tracking-alt"),
    key_stop_tracking1(b, "stop-tracking"),
    key_stop_tracking2(b, "stop-tracking-alt"),
    key_toggle_tracking1(b, "toggle-tracking"),
    key_toggle_tracking2(b, "toggle-tracking-alt"),
    key_restart_tracking1(b, "restart-tracking"),
    key_restart_tracking2(b, "restart-tracking-alt"),
    key_center1(b, "center"),
    key_center2(b, "center-alt"),
    key_toggle1(b, "toggle"),
    key_toggle2(b, "toggle-alt"),
    key_zero1(b, "zero"),
    key_zero2(b, "zero-alt"),
    key_toggle_press1(b, "toggle-press"),
    key_toggle_press2(b, "toggle-press-alt"),
    key_zero_press1(b, "zero-press"),
    key_zero_press2(b, "zero-press-alt"),
    tracklogging_enabled(b, "tracklogging-enabled", false),
    tracklogging_filename(b, "tracklogging-filename", QString())
{
}

module_settings::module_settings() :
    b(make_bundle("modules")),
    tracker_dll(b, "tracker-dll", "PointTracker 1.1"),
    filter_dll(b, "filter-dll", "Accela"),
    protocol_dll(b, "protocol-dll", "freetrack 2.0 Enhanced")
{
}

key_opts::key_opts(bundle b, const QString& name) :
    keycode(b, QString("keycode-%1").arg(name), ""),
    guid(b, QString("guid-%1").arg(name), ""),
    button(b, QString("button-%1").arg(name), -1)
{}


axis_opts::axis_opts(bundle b_settings_window, bundle b_mapping_window, QString pfx, Axis idx) :
    b_settings_window(b_settings_window),
    b_mapping_window(b_mapping_window),
    zero(b_settings_window, n(pfx, "zero-pos"), 0),
    src(b_settings_window, n(pfx, "source-index"), idx),
    invert(b_settings_window, n(pfx, "invert-sign"), false),
    altp(b_mapping_window, n(pfx, "alt-axis-sign"), false),
    clamp(b_mapping_window, n(pfx, "max-value"), idx >= Yaw ? r180 : t30)
{}

QString axis_opts::n(QString pfx, QString name)
{
    return QString("%1-%2").arg(pfx, name);
}
