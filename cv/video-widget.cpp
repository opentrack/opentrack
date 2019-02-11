#include "video-widget.hpp"

#include <opencv2/imgproc.hpp>

void cv_video_widget::update_image(const cv::Mat& frame)
{
    QMutexLocker l(&mtx);

    if (freshp)
        return;

    if (W < 1 || H < 1 || frame.rows < 1 || frame.cols < 1)
        return;

    cv::Mat const* __restrict frame_scaled = nullptr;

    if (frame3.cols != W || frame3.rows != H)
    {
        frame3 = cv::Mat(H, W, frame.type());
        frame2 = cv::Mat(H, W, CV_8UC4);

        if (!frame2.isContinuous() || !frame3.isContinuous())
            std::abort();
    }

    if (frame.cols != W || frame.rows != H)
    {
        cv::resize(frame, frame3, { W, H }, 0, 0, cv::INTER_NEAREST);
        frame_scaled = &frame3;
    }
    else if (!frame.isContinuous())
    {
        frame.copyTo(frame3);
        frame_scaled = &frame3;
    }
    else
        frame_scaled = &frame;

    freshp = true;

    int color_cvt = 0;
    constexpr int nchannels = 4;

    switch (frame_scaled->channels())
    {
    case 1:
        color_cvt = cv::COLOR_GRAY2BGRA;
        break;
    case 3:
        color_cvt = cv::COLOR_BGR2BGRA;
        break;
    case nchannels:
        break;
    default:
        unreachable();
        break;
    }

    cv::Mat const* frame_color;

    if (color_cvt != cv::COLOR_COLORCVT_MAX)
    {
        cv::cvtColor(*frame_scaled, frame2, color_cvt);
        frame_color = &frame2;
    }
    else
        frame_color = frame_scaled;

    int stride = frame_color->step.p[0], rows = frame_color->rows;
    int nbytes = rows * stride;
    vec.resize(nbytes); vec.shrink_to_fit();
    std::memcpy(vec.data(), frame_color->data, nbytes);

    texture = QImage((const unsigned char*) vec.data(), W, H, stride, QImage::Format_ARGB32);
    texture.setDevicePixelRatio(devicePixelRatioF());
}

cv_video_widget::cv_video_widget(QWidget* parent) : video_widget(parent) {}
