#include "trackhat.hpp"
#include <opencv2/imgproc.hpp>
#include "compat/math.hpp"

trackhat_preview::trackhat_preview(int w, int h)
{
    frame_bgr.create(h, w, CV_8UC3);
    frame_bgra.create(h, w, CV_8UC4);
}

void trackhat_preview::set_last_frame(const pt_frame& frame_)
{
    center = {-1, -1};
    points = frame_.as_const<trackhat_frame>()->points;
}

void trackhat_preview::draw_head_center(pt_pixel_pos_mixin::f x, pt_pixel_pos_mixin::f y)
{
    center = {x, y};
}

QImage trackhat_preview::get_bitmap()
{
    frame_bgr.setTo({0});

    draw_points();
    draw_center();

    cv::cvtColor(frame_bgr, frame_bgra, cv::COLOR_BGR2BGRA);

    return QImage((const unsigned char*) frame_bgra.data,
                  frame_bgra.cols, frame_bgra.rows,
                  (int)frame_bgra.step.p[0],
                  QImage::Format_ARGB32);
}

void trackhat_preview::draw_center()
{
    if (center == numeric_types::vec2(-1, -1))
        return;

    auto [px_, py_] = to_pixel_pos(center[0], center[1], frame_bgr.cols, frame_bgr.rows);
    int px = iround(px_), py = iround(py_);

    const f dpi = (f)frame_bgr.cols / f(320);
    constexpr int len_ = 9;
    int len = iround(len_ * dpi);

    static const cv::Scalar color(0, 255, 255);
    cv::line(frame_bgr,
             cv::Point(px - len, py),
             cv::Point(px + len, py),
             color, 1);
    cv::line(frame_bgr,
             cv::Point(px, py - len),
             cv::Point(px, py + len),
             color, 1);
}

void trackhat_preview::draw_points()
{
    for (unsigned i = 0; i < std::size(points.m_point); i++)
    {
        const auto pt = points.m_point[i];

        if (pt.m_brightness == 0)
            continue;

        constexpr f sz = trackhat_camera::sensor_size;
        f x = std::clamp((f)pt.m_x, f(0), sz-1) * (f)frame_bgr.cols / sz,
          y = std::clamp((f)pt.m_y, f(0), sz-1) * (f)frame_bgr.rows / sz;

        const f dpi = (f)frame_bgr.cols / f(320);
        int c = (int)pt.m_brightness;
        constexpr int point_size = 6;
        auto outline_color = i < 3 ? cv::Scalar{255, 255, 0} : cv::Scalar{192, 192, 192};

        cv::circle(frame_bgr,
                   {iround(x*dpi), iround(y*dpi)},
                   iround(point_size * dpi),
                   outline_color,
                   iround(dpi), cv::LINE_AA);

        cv::circle(frame_bgr,
                   {iround(x*dpi), iround(y*dpi)},
                   iround((point_size-2) * dpi),
                   cv::Scalar(c, c, c),
                   -1);
    }
}
