#include "trackhat.hpp"

namespace trackhat_impl {

trackhat_settings::trackhat_settings() : opts{"tracker-trackhat"}
{
}

void setting_receiver::settings_changed()
{
    changed = true;
}

bool setting_receiver::test_and_clear()
{
    bool x = true;
    return changed.compare_exchange_strong(x, false);
}

} // ns trackhat_impl

void trackhat_camera::set_pt_options()
{
    s.min_point_size = t.min_pt_size;
    s.max_point_size = t.max_pt_size;

    switch (t.model)
    {
    default:
    case model_cap:
        s.t_MH_x = 0; s.t_MH_y = 0; s.t_MH_z = 0;
        break;
    case model_mystery_meat:
    case model_clip_left:
    case model_mini_clip_left:
        s.t_MH_x = -135; s.t_MH_y = 0; s.t_MH_z = 0;
        break;
    case model_clip_right:
    case model_mini_clip_right:
        s.t_MH_x = 135; s.t_MH_y = 0; s.t_MH_z = 0;
        break;
    }

    switch (t.model)
    {
    default:
        eval_once(qDebug() << "tracker/trackhat: unknown model");
    [[fallthrough]];
    case model_clip_left:
    case model_clip_right:
        s.clip_tz = 27; s.clip_ty = 43; s.clip_by = 62; s.clip_bz = 74;
        break;
    case model_mini_clip_left:
    case model_mini_clip_right:
        s.clip_tz = 13; s.clip_ty = 42; s.clip_by = 60; s.clip_bz = 38;
        break;
    case model_cap:
        s.cap_x = 60; s.cap_y = 90; s.cap_z = 95;
        break;
    case model_mystery_meat:
        s.clip_tz = 15; s.clip_ty = 40; s.clip_by = 70; s.clip_bz = 40;
        break;
    }

    s.camera_name = "TrackHat Sensor (WIP)";

    s.active_model_panel = t.model == model_cap ? 1 : 0;
    s.enable_point_filter = t.enable_point_filter;
    s.point_filter_coefficient = *t.point_filter_coefficient;
}
