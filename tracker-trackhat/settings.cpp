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
        break;
    }

    s.dynamic_pose = t.model == model_cap;
    s.init_phase_timeout = 500;

    s.camera_name = "TrackHat Sensor (WIP)";

    s.active_model_panel = t.model == model_cap ? 1 : 0;
    s.enable_point_filter = t.enable_point_filter;
    s.point_filter_coefficient = *t.point_filter_coefficient;
}

bool trackhat_camera::set_regs(const uint8_t(*regs)[3], unsigned len)
{
    unsigned attempts = 0;
    constexpr unsigned max_attempts = 5;

start:
    for (unsigned i = 0; i < len; i++)
    {
        const auto& reg = regs[i];
        trackHat_SetRegister_t r{reg[0], reg[1], reg[2]};
        auto status = th_check(trackHat_SetRegisterValue(&*device, &r));
        if (status == TH_SUCCESS)
            continue;
        else if (status == TH_FAILED_TO_SET_REGISTER)
            goto retry;
        else if (status == TH_ERROR_DEVICE_COMUNICATION_TIMEOUT)
            goto retry;
        else
            goto error;
    }

    return true;

retry:
    if (attempts++ < max_attempts)
    {
        auto dbg = qDebug();
        dbg << "tracker/trackhat: set register retry attempt";
        dbg.space(); dbg.nospace();
        dbg << attempts << "/" << max_attempts;
        portable::sleep(50);
        goto start;
    }

error:
    return false;
}

bool trackhat_camera::init_regs()
{
    auto exp    = (uint8_t)t.exposure;
    auto exp2   = (uint8_t)(exp == 0xff ? 0xf0 : 0xff);
    auto thres  = (uint8_t)t.threshold;
    auto thres2 = (uint8_t)std::clamp((int)*t.threshold_2, 0, std::max(64, thres-1));

    auto gain   = (uint8_t)((int)*t.gain);
    auto gain_c = (uint8_t)(gain/0x10);
    gain %= 0x10; gain_c %= 4;

    const uint8_t regs[][3] = {
        { 0x0c, 0x0f, exp2   },  // exposure lo
        { 0x0c, 0x10, exp    },  // exposure hi
        { 0x00, 0x0b, 0xff   },  // blob area max size
        { 0x00, 0x0c, 0x03   },  // blob area min size
        { 0x0c, 0x08, gain   },  // gain
        { 0x0c, 0x0c, gain_c },  // gain multiplier
        { 0x0c, 0x47, thres  },  // min brightness
        { 0x00, 0x0f, thres2 },  // brightness margin, formula is `thres >= px > thres - fuzz'
        { 0x00, 0x01, 0x01   },  // bank0 sync
        { 0x01, 0x01, 0x01   },  // bank1 sync
    };

    Timer t;

    if (!set_regs(regs, std::size(regs)))
    {
        device.disconnect();
        return false;
    }

    if (int elapsed = (int)t.elapsed_ms(); elapsed > 500)
        qDebug() << "tracker/trackhat: setting registers took" << elapsed << "ms";

    return true;
}
