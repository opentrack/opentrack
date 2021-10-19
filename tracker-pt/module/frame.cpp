#include "frame.hpp"
#include "compat/math.hpp"
#include <opencv2/imgproc.hpp>

namespace pt_module {

void Preview::set_last_frame(const pt_frame& frame_)
{
    const cv::Mat& frame = frame_.as_const<const Frame>()->mat;
    const bool need_resize = frame.size != frame_copy.size;

    if (frame.channels() == 1)
    {
        if (need_resize)
        {
            frame_tmp.create(frame.size(), CV_8UC3);
            cv::cvtColor(frame, frame_tmp, cv::COLOR_GRAY2BGR);
            cv::resize(frame_tmp, frame_copy, frame_copy.size(), 0, 0, cv::INTER_NEAREST);
        }
        else
            cv::cvtColor(frame, frame_copy, cv::COLOR_GRAY2BGR);
    }
    else if (frame.channels() == 3)
    {
        if (need_resize)
            cv::resize(frame, frame_copy, frame_copy.size(), 0, 0, cv::INTER_NEAREST);
        else
            frame.copyTo(frame_copy);
    }
    else
    {
        eval_once(qDebug() << "tracker/pt: camera frame depth" << frame.channels() << "!= 3");
        frame_copy.create(frame_copy.size(), CV_8UC3);
        frame_copy.setTo({0});
    }
}

Preview::Preview(int w, int h)
{
    frame_out.create(h, w, CV_8UC4);
    frame_copy.create(h, w, CV_8UC3);
    frame_copy.setTo({0});
}

QImage Preview::get_bitmap()
{
    int stride = (int)frame_out.step.p[0];

    if (stride < frame_out.cols * 4)
    {
        eval_once(qDebug() << "bad stride" << stride
                           << "for bitmap size" << frame_copy.cols << frame_copy.rows);
        return QImage();
    }

    cv::cvtColor(frame_copy, frame_out, cv::COLOR_BGR2BGRA);

    return QImage((const unsigned char*) frame_out.data,
                  frame_out.cols, frame_out.rows,
                  stride,
                  QImage::Format_ARGB32);
}

void Preview::draw_head_center(f x, f y)
{
    auto [px_, py_] = to_pixel_pos(x, y, frame_copy.cols, frame_copy.rows);

    int px = iround(px_), py = iround(py_);

    constexpr int len = 9;

    static const cv::Scalar color(0, 255, 255);
    cv::line(frame_copy,
             cv::Point(px - len, py),
             cv::Point(px + len, py),
             color, 1);
    cv::line(frame_copy,
             cv::Point(px, py - len),
             cv::Point(px, py + len),
             color, 1);
}

} // ns pt_module
