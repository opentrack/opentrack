#include "logic/main-settings.hpp"
#include "logic/mappings.hpp"

#include "tracker-pt/pt-settings.hpp"
#include "filter-accela/accela-settings.hpp"
#include "proto-mouse/mouse-settings.hpp"

#include "options/options.hpp"

#include <QSettings>

using namespace options;

static void force_spline_settings()
{
    main_settings main;

    axis_opts** all_axis_opts = main.all_axis_opts;
    Mappings mappings { all_axis_opts };

    for (unsigned k = 0; k < 6; k++)
    {
        Map& map = mappings(k);
        const QString& prefix = all_axis_opts[k]->prefix();

        const QString& name1 = map.name;
        const QString& name2 = map.alt_name;

        bundle b = make_bundle(prefix);

        spline_detail::settings s1(b, name1, Axis(k));
        spline_detail::settings s2(b, name2, Axis(k));

        s1.points = QList<QPointF> { { 180, 180 } };
        s2.points = QList<QPointF> { { 180, 180 } };

        b->save();
    }
}

static void force_main_settings()
{
    main_settings s;
    s.center_at_startup = true;
    s.reltrans_mode = reltrans_disabled;
    s.neck_enable = false;

    module_settings m;

    m.tracker_dll = "pt";
    m.protocol_dll = "win32-mouse";
    m.filter_dll = "accela";

    s.b->save();
    s.b_map->save();
}

static void force_pt_settings()
{
    pt_settings s("tracker-pt");

    enum { Clip = 0 };

    s.active_model_panel = Clip;
    // XXX TODO these are Mini Clip Right sizes
    s.clip_by = 60;
    s.clip_bz = 38.2;
    s.clip_ty = 42.2;
    s.clip_tz = 12.6;

    s.cam_fps = 60;
    s.cam_res_x = 640;
    s.cam_res_y = 480;
    s.camera_name = "PS3Eye Camera";

    s.min_point_size = 6;
    s.max_point_size = 15;

    // XXX TODO auto threshold slider position
    s.auto_threshold = true;
    s.threshold_slider = slider_value(178., s.threshold_slider->min(), s.threshold_slider->max());

    s.t_MH_x = 0, s.t_MH_y = 0, s.t_MH_z = 0;
    s.blob_color = pt_color_natural;
    s.fov = 56;
    s.dynamic_pose = false;

    s.b->save();
}

static void force_mouse_settings()
{

    mouse_settings s;

    s.Mouse_X = Yaw + 1;
    s.Mouse_Y = Pitch + 1;

    s.b->save();
}

static void force_accela_settings()
{
    // TODO
}

static void force_shortcut_settings()
{
    main_settings s;
    s.key_toggle_tracking1.keycode = "Alt+F10";
    s.key_center1.keycode = "Alt+F11";
    s.key_toggle_press1.keycode = "Alt+F12";

    for (key_opts* k : { &s.key_toggle_tracking1, &s.key_center1, &s.key_toggle_press1 })
    {
        k->button = -1;
        k->guid = {};
    }
    s.b->save();
}

void force_trackmouse_settings()
{
   options::globals::with_settings_object([](QSettings&) {
       force_main_settings();
       force_spline_settings();
       force_pt_settings();
       force_mouse_settings();
       force_accela_settings();
       force_shortcut_settings();
   });
}
