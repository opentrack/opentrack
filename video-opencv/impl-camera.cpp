#include "impl.hpp"
#include "compat/sleep.hpp"
#include "video-property-page.hpp"
#include <QDebug>

namespace opencv_camera_impl {

cam::cam(int idx) : idx(idx)
{
}

cam::~cam()
{
    stop();
}

void cam::stop()
{
    if (cap)
    {
        if (cap->isOpened())
            cap->release();
        cap = std::nullopt;
    }
    mat = cv::Mat();
    frame_ = { {}, false };
}

bool cam::is_open()
{
    return !!cap;
}

void cam::set_exposure(bool write)
{
    auto e = *s.exposure;
    if (e != exposure)
        switch (e)
        {
            case exposure_preset::near: cap->set(cv::CAP_PROP_EXPOSURE, -5); qDebug() << "near"; break;
            case exposure_preset::far: cap->set(cv::CAP_PROP_EXPOSURE, -4); qDebug() << "far"; break;
            default: break;
        }

    if (s.exposure != exposure_preset::ignored)
    {
        constexpr struct {
            int prop, value;
        } props[] = {
            { cv::CAP_PROP_AUTO_EXPOSURE,   0 },
            { cv::CAP_PROP_BRIGHTNESS,      0 },
            { cv::CAP_PROP_SHARPNESS,       3 },
            { cv::CAP_PROP_AUTO_EXPOSURE,   0 },
            { cv::CAP_PROP_BRIGHTNESS,      0 },
            { cv::CAP_PROP_SHARPNESS,       3 },
            { cv::CAP_PROP_CONTRAST,       32 },
            { cv::CAP_PROP_HUE,             0 },
            { cv::CAP_PROP_SATURATION,      0 },
            { cv::CAP_PROP_SHARPNESS,       3 },
            { cv::CAP_PROP_GAMMA,         100 },
            { cv::CAP_PROP_BACKLIGHT,       1 },
            { cv::CAP_PROP_GAIN,            0 },
        };
        for (const auto [prop, value] : props)
            cap->set(prop, value);
    }

    if (write)
        exposure = e;
}

bool cam::start(info& args)
{
    stop();
    cap.emplace(idx, video_capture_backend);

    if (args.width > 0 && args.height > 0)
    {
        cap->set(cv::CAP_PROP_FRAME_WIDTH,  args.width);
        cap->set(cv::CAP_PROP_FRAME_HEIGHT, args.height);
    }
    if (args.fps > 0)
        cap->set(cv::CAP_PROP_FPS, args.fps);

    if (args.use_mjpeg)
        cap->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));

    set_exposure(false);

    if (!cap->isOpened())
        goto fail;

    if (!get_frame_())
        goto fail;

    return true;

fail:
    stop();
    return false;
}

bool cam::get_frame_()
{
    if (!is_open())
        return false;

    for (unsigned i = 0; i < 10; i++)
    {
        if (cap->read(mat))
        {
            frame_.data = mat.data;
            frame_.width = mat.cols;
            frame_.height = mat.rows;
            frame_.stride = mat.step.p[0];
            if (mat.step.p[0] == (unsigned)frame_.width * mat.elemSize())
                frame_.stride = cv::Mat::AUTO_STEP;

            frame_.channels = mat.channels();

            return true;
        }
        portable::sleep(50);
    }

    set_exposure(true);

    return false;
}

std::tuple<const frame&, bool> cam::get_frame()
{
    bool ret = get_frame_();
    return { frame_, ret };
}

bool cam::show_dialog()
{
    if (is_open())
        return video_property_page::show_from_capture(*cap, idx);
    else
        return video_property_page::show(idx);
}

} // ns opencv_camera_impl
