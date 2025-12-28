#include "video-widget.hpp"
#include "compat/macros.h"
#include <opencv2/imgproc.hpp>

void cv_video_widget::update_image(const cv::Mat& frame)
{
    if (fresh())
        return;

    auto [ W, H ] = preview_size();

    if (W < 1 || H < 1 || frame.rows < 1 || frame.cols < 1)
        return;

    cv::Mat const* __restrict scaled = nullptr;
    frame3.create(H, W, frame.type());
    frame2.create(H, W, CV_8UC4);

    if (frame.cols != W || frame.rows != H)
    {
        cv::resize(frame, frame3, { W, H }, 0, 0, cv::INTER_NEAREST);
        scaled = &frame3;
    }
    else if (!frame.isContinuous())
    {
        frame.copyTo(frame3);
        scaled = &frame3;
    }
    else
        scaled = &frame;

    int color_cvt = cv::COLOR_COLORCVT_MAX;

    switch (scaled->channels())
    {
    case 1:
        color_cvt = cv::COLOR_GRAY2BGRA;
        break;
    case 3:
        color_cvt = cv::COLOR_BGR2BGRA;
        break;
    case 4:
        break;
    default:
        tr_unreachable();
        break;
    }

    cv::Mat const* color;

    if (color_cvt != cv::COLOR_COLORCVT_MAX)
    {
        cv::cvtColor(*scaled, frame2, color_cvt);
        color = &frame2;
    }
    else
        color = scaled;

    int width = color->cols, height = color->rows;
    unsigned stride = (unsigned)color->step.p[0];
    set_image(color->data, width, height, stride, QImage::Format_ARGB32);

    set_fresh(true);
}
