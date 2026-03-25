#include "impl.hpp"
#include "compat/sleep.hpp"
#include "video-property-page.hpp"

#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <QDebug>
#include <QString>
#endif

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

#ifdef __linux__
static bool force_v4l2_fps(int camera_idx, int fps)
{
    if (fps <= 0)
        return true;

    QString dev_path = QString("/dev/video%1").arg(camera_idx);
    int fd = open(dev_path.toUtf8().constData(), O_RDWR);
    if (fd < 0)
    {
        qDebug() << "video/opencv: v4l2 can't open" << dev_path;
        return false;
    }

    struct v4l2_streamparm parm = {};
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(fd, VIDIOC_G_PARM, &parm) < 0)
    {
        qDebug() << "video/opencv: VIDIOC_G_PARM failed";
        close(fd);
        return false;
    }

    parm.parm.capture.timeperframe.numerator = 1;
    parm.parm.capture.timeperframe.denominator = fps;

    if (ioctl(fd, VIDIOC_S_PARM, &parm) < 0)
    {
        qDebug() << "video/opencv: VIDIOC_S_PARM failed for" << fps << "fps";
        close(fd);
        return false;
    }

    ioctl(fd, VIDIOC_G_PARM, &parm);
    int actual_fps = parm.parm.capture.timeperframe.denominator /
                     (parm.parm.capture.timeperframe.numerator
                      ? parm.parm.capture.timeperframe.numerator : 1);

    qDebug() << "video/opencv: v4l2 framerate set to" << actual_fps << "fps"
             << "(requested" << fps << "fps)";

    close(fd);
    return actual_fps == fps;
}
#endif

bool cam::start(info& args)
{
    stop();
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_WARNING);
    cap.emplace(idx, video_capture_backend);

#if !defined _WIN32 && !defined __APPLE__
    if (args.use_mjpeg)
        cap->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
#endif

    if (args.width > 0 && args.height > 0)
    {
        cap->set(cv::CAP_PROP_FRAME_WIDTH,  args.width);
        cap->set(cv::CAP_PROP_FRAME_HEIGHT, args.height);
    }
    if (args.fps > 0)
        cap->set(cv::CAP_PROP_FPS, args.fps);

#if defined _WIN32 || defined __APPLE__
    if (args.use_mjpeg)
        cap->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
#endif

#ifdef __linux__
    force_v4l2_fps(idx, args.fps);
#endif

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
