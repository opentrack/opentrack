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

trackhat_camera::trackhat_camera() = default;

trackhat_camera::~trackhat_camera()
{
    stop();
}

pt_camera::result trackhat_camera::get_frame(pt_frame& frame_)
{
    auto& ret = *frame_.as<trackhat_frame>();
    trackhat_frame frame;

    if (status < th_running || error_code != TH_SUCCESS)
    {
        if (status >= th_running)
            qDebug() << "trackhat: disconnected, status" << (void*)error_code;
        goto error;
    }

    if (TH_ErrorCode error = trackHat_GetDetectedPoints(&device, &frame.points); error != TH_SUCCESS)
    {
        error_code = error;
        goto error;
    }

    ret.points = frame.points;
    return {true, get_desired()};

error:
    if (error_code != TH_SUCCESS)
        qDebug() << "trackhat: error" << (void*)error_code;
    ret.points = {};
    stop();
    return {false, get_desired()};
}

#define CHECK(x)                                                \
    do {                                                        \
        if (TH_ErrorCode status_ = (x); status_ != TH_SUCCESS)  \
        {                                                       \
            qDebug() << "trackhat: error"                       \
                     << (void*)status_ << "in" << #x;           \
            error_code = status_;                               \
            goto error;                                         \
        }                                                       \
    } while (false)

bool trackhat_camera::start(const pt_settings&)
{
    int attempts = 0;

    if (status >= th_running)
        return true;

start:
    stop();
    trackHat_EnableDebugMode();

    [[maybe_unused]] uint32_t uptime = 0;
    error_code = TH_SUCCESS;
    status = th_noinit;

    CHECK(trackHat_Initialize(&device)); status = th_init;
    CHECK(trackHat_DetectDevice(&device)); status = th_detect;
    CHECK(trackHat_Connect(&device)); status = th_connect;
    CHECK(trackHat_GetUptime(&device, &uptime)); status = th_running;

#if 0
    qDebug() << "trackhat start: device uptime" << uptime << "seconds";
#endif

    return true;
error:
    stop();
    if (++attempts < 5)
    {
        portable::sleep(100);
        goto start;
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

