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
