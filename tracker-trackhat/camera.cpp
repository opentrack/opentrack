#include "trackhat.hpp"
#include "compat/sleep.hpp"
#include <cstdio>

namespace trackhat_impl {

TH_ErrorCode log_error(TH_ErrorCode error, const char* source)
{
    if (error == TH_ERROR_DEVICE_ALREADY_OPEN)
        error = TH_SUCCESS;
    if (error)
        fprintf(stderr, "error 0x%x in %s\n", -error, source);
    fflush(stderr);
    return error;
}

} // ns trackhat_impl

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
    if (!device.ensure_connected())
        goto error;

    if (sig.test_and_clear())
    {
        set_pt_options();
        if (!init_regs())
            goto error;
    }

    {
        trackHat_ExtendedPoints_t points;
        if (!!th_check(trackHat_GetDetectedPointsExtended(&*device, &points)))
            goto error;
        auto& frame = *frame_.as<trackhat_frame>();
        frame.init_points(points, t.min_pt_size, t.max_pt_size);
    }

    return {true, get_desired()};

error:
    stop();
    return {false, {}};
}

static void log_handler(const char* file, int line, const char* function, char level, const char* str, size_t len)
{
    if (level != 'E')
        return;
    char file_[128];
    snprintf(file_, std::size(file_), "trackhat/%s", file);
    auto logger = QMessageLogger(file_, line, function).debug();
    logger << "tracker/trackhat:";
    logger.noquote() << QLatin1String(str, (int)len);
}

bool trackhat_camera::start(const pt_settings&)
{
    trackHat_SetDebugHandler(log_handler);

    if constexpr(debug_mode)
        trackHat_EnableDebugMode();
    else
        trackHat_DisableDebugMode();

    if (!device.ensure_device_exists())
        return false;

    set_pt_options();

    return true;
}

void trackhat_camera::stop()
{
    device.disconnect();
}
