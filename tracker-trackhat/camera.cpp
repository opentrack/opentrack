#include "trackhat.hpp"
#include "compat/sleep.hpp"
#include <QDebug>

pt_camera::result trackhat_camera::get_info() const
{
    return {true, get_desired() };
}

pt_camera_info trackhat_camera::get_desired() const
{
    pt_camera_info ret = {};

    ret.fov = sensor_fov;
    ret.fps = 250;
    ret.res_x = sensor_size;
    ret.res_y = sensor_size;

    return ret;
}

QString trackhat_camera::get_desired_name() const
{
    return QStringLiteral("TrackHat sensor");
}

QString trackhat_camera::get_active_name() const
{
    return get_desired_name();
}

void trackhat_camera::set_fov(pt_camera::f) {}
void trackhat_camera::show_camera_settings() {}

trackhat_camera::trackhat_camera()
{
    s.set_raii_dtor_state(false);
    t.set_raii_dtor_state(false);

    for (auto* slider : { &t.exposure, &t.gain, &t.threshold, &t.threshold_2 })
    {
        QObject::connect(slider, options::value_::value_changed<options::slider_value>(),
                         &sig, &trackhat_impl::setting_receiver::settings_changed,
                         Qt::DirectConnection);
    }
}

trackhat_camera::~trackhat_camera()
{
    stop();
}

pt_camera::result trackhat_camera::get_frame(pt_frame& frame_)
{
start:
    if (status < th_running || error_code != TH_SUCCESS)
        goto error;

    if (sig.test_and_clear())
    {
        set_pt_options();
        if ((error_code = (decltype(error_code))init_regs()) != TH_SUCCESS)
            goto error;
    }

    if (trackHat_ExtendedPoints_t points = {};
        (error_code = trackHat_GetDetectedPointsExtended(&device, &points)) == TH_SUCCESS)
    {
        auto& frame = *frame_.as<trackhat_frame>();
        frame.init_points(points, t.min_pt_size, t.max_pt_size);
    }
    else
        goto error;

    return {true, get_desired()};

error:
    if (status >= th_running)
    {
        qDebug() << "trackhat: error" << (void*)error_code;
    }
    stop();
    if (start(s))
        goto start;
    return {false, get_desired()};
}

#define CHECK(x)                                                \
    do {                                                        \
        if (TH_ErrorCode status_ = (x); status_ != TH_SUCCESS)  \
        {                                                       \
            qDebug() << "trackhat: error"                       \
                     << (void*)-status_ << "in" << #x;          \
            error_code = status_;                               \
            goto error;                                         \
        }                                                       \
    } while (false)

int trackhat_camera::init_regs()
{
    unsigned attempts = 0;
    constexpr unsigned max_attempts = 5;
    TH_ErrorCode error;

    auto exp = (uint8_t)t.exposure;
    auto thres = (uint8_t)t.threshold;
    auto thres2 = (uint8_t)std::clamp((int)*t.threshold_2, 0, std::max(64, thres-1));

    auto gain = (uint8_t)*t.gain;
    auto gain_c = (uint8_t)((gain/0x0f + !!(gain/0x0f)) & 3);
    gain %= 0x0f;

    const uint8_t regs[][3] = {
        { 0x0c, 0x0f, 0xf0   },  // exposure lo
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

start:
    for (const auto& reg : regs)
    {
        trackHat_SetRegister_t r{reg[0], reg[1], reg[2]};
        error = trackHat_SetRegisterValue(&device, &r);
        if (error != TH_SUCCESS)
            goto error;
    }

    return TH_SUCCESS;

error:
    if (attempts++ < max_attempts)
    {
        portable::sleep(50);
        goto start;
    }

    return error;
}

bool trackhat_camera::start(const pt_settings&)
{
    int attempts = 0;
    constexpr int max_attempts = 5;

    set_pt_options();

    if (status >= th_running)
        return true;

start:
    stop();
    trackHat_DisableDebugMode();

    [[maybe_unused]] uint32_t uptime = 0;
    error_code = TH_SUCCESS;
    status = th_noinit;

    CHECK(trackHat_Initialize(&device)); status = th_init;
    CHECK(trackHat_DetectDevice(&device)); status = th_detect;
    CHECK(trackHat_Connect(&device, TH_FRAME_EXTENDED)); status = th_connect;
    CHECK(trackHat_GetUptime(&device, &uptime));
    CHECK((TH_ErrorCode)init_regs()); status = th_running;

    return true;
error:
    stop();
    switch (error_code)
    {
    case TH_ERROR_DEVICE_NOT_DETECTED:
    case TH_ERROR_CAMERA_SELFT_TEST_FAILD:
    case TH_ERROR_CAMERA_INTERNAL_BROKEN:
        break;
    default:
        if (attempts++ < max_attempts)
        {
            portable::sleep(10);
            goto start;
        }
    }
    return false;
}

void trackhat_camera::stop()
{
#if 0
    if (status >= th_connect)
    {
        uint32_t uptime = 0;
        if (TH_ErrorCode status = trackHat_GetUptime(&device, &uptime); status == TH_SUCCESS)
            qDebug() << "trackhat stop: device uptime" << uptime << "seconds";
    }
#endif

    if (status >= th_connect)
        (void)trackHat_Disconnect(&device);
    if (status >= th_init)
        (void)trackHat_Deinitialize(&device);

    status = th_noinit;
    device = {};
}
