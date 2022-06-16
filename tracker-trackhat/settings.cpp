#include "trackhat.hpp"
#include "compat/sleep.hpp"
#include "compat/timer.hpp"

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

setting_receiver::setting_receiver(bool value) : changed{value}
{
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
        break;
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
        s.active_model_panel = 0;
        break;
    case model_mini_clip_left:
    case model_mini_clip_right:
        s.clip_tz = 13; s.clip_ty = 42; s.clip_by = 60; s.clip_bz = 38;
        s.active_model_panel = 0;
        break;
    case model_cap:
        s.cap_x = 60; s.cap_y = 90; s.cap_z = 95;
        s.active_model_panel = 1;
        break;
    case model_mystery_meat:
        break;
    }

    s.dynamic_pose = t.model == model_cap;
    s.init_phase_timeout = 500;

    s.camera_name = QStringLiteral("TrackHat Sensor");

    s.enable_point_filter = t.enable_point_filter;
    s.point_filter_coefficient = *t.point_filter_coefficient;
    s.point_filter_limit = *t.point_filter_limit;
    s.point_filter_deadzone = *t.point_filter_deadzone;
}

bool trackhat_camera::init_regs()
{
    auto exp    = (uint8_t)t.exposure;
    auto exp2   = (uint8_t)(exp == 0xff ? 0xf0 : 0xff);
    auto thres  = (uint8_t)0xfe;
    auto thres2 = (uint8_t)3;

    auto gain   = (uint8_t)t.gain;
    auto gain_c = (uint8_t)(gain/0x10);
    gain %= 0x10; gain_c %= 4;

    trackHat_SetRegisterGroup_t regs = {
        {
            { 0x0c, 0x0f, exp2   },  // exposure lo
            { 0x0c, 0x10, exp    },  // exposure hi
            { 0x00, 0x0b, 0xff   },  // blob area max size
            { 0x00, 0x0c, 0x03   },  // blob area min size
            { 0x0c, 0x08, gain   },  // gain
            { 0x0c, 0x0c, gain_c },  // gain multiplier
            { 0x0c, 0x47, thres  },  // min brightness
            { 0x00, 0x0f, thres2 },  // brightness margin, formula is `thres >= px > thres - fuzz'
            { 0x0c, 0x60, 0xff   },  // scale resolution lo
            { 0x0c, 0x61, 0x0f   },  // scale resolution hi
            { 0x00, 0x01, 0x01   },  // bank0 sync
            { 0x01, 0x01, 0x01   },  // bank1 sync
        },
        12,
    };

    Timer t;

    constexpr int max = 5;
    int i = 0;
    for (i = 0; i < max; i++)
    {
        TH_ErrorCode status = TH_SUCCESS;
        status = th_check(trackHat_SetRegisterGroupValue(&*device, &regs));
        if (status == TH_SUCCESS)
            break;
        else if (status != TH_FAILED_TO_SET_REGISTER &&
                 status != TH_ERROR_DEVICE_COMMUNICATION_TIMEOUT)
            return false;
        else
        {
            auto dbg = qDebug();
            dbg << "tracker/trackhat: set register retry attempt";
            dbg.space(); dbg.nospace();
            dbg << i << "/" << max;
            portable::sleep(50);
        }
    }

    if (i == max)
        return false;

    if (int elapsed = (int)t.elapsed_ms(); elapsed > 100)
        qDebug() << "tracker/trackhat: setting registers took" << elapsed << "ms";

    return true;
}
