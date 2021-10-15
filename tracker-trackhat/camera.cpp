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
    trackHat_ExtendedPoints_t points = {};

    if (status < th_running || error_code != TH_SUCCESS)
        goto error;

    if (TH_ErrorCode error = trackHat_GetDetectedPointsExtended(&device, &points); error != TH_SUCCESS)
    {
        error_code = error;
        goto error;
    }

    ret.init_points(points, s.min_point_size, s.max_point_size);
    return {true, get_desired()};

error:
    if (status >= th_running)
        qDebug() << "trackhat: error" << (void*)error_code;
    ret.init_points(points, s.min_point_size, s.max_point_size);
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

int trackhat_camera::init_regs()
{
    constexpr uint8_t regs[][3] = {
        { 0x0c, 0x0f, 0xf0 },  // exposure lo
        { 0x0c, 0x10, 0x7f },  // exposure hi
        { 0x01, 0x01, 0x01 },  // bank1 sync
    };

    for (const auto& reg : regs)
    {
        trackHat_SetRegister_t r{reg[0], reg[1], reg[2]};
        if (TH_ErrorCode error = trackHat_SetRegisterValue(&device, &r); error != TH_SUCCESS)
            return error;
    }

    return TH_SUCCESS;
}

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
    CHECK(trackHat_Connect(&device, TH_FRAME_EXTENDED)); status = th_connect;
    CHECK(trackHat_GetUptime(&device, &uptime));
    CHECK((TH_ErrorCode)init_regs()); status = th_running;

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
