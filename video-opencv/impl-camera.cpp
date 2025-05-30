#include "impl.hpp"
#include "compat/sleep.hpp"
#include "video-property-page.hpp"
#include <opencv2/core/utils/logger.hpp>

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

bool cam::start(info& args)
{
    stop();
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_WARNING);
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
            frame_.stride = (int)mat.step.p[0];
            if (mat.step.p[0] == (unsigned)frame_.width * mat.elemSize())
                frame_.stride = cv::Mat::AUTO_STEP;

            frame_.channels = mat.channels();

            return true;
        }
        portable::sleep(50);
    }

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
