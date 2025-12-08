#include "preview.h"

namespace neuralnet_tracker_ns
{

cv::Rect make_crop_rect_for_aspect(const cv::Size& size, int aspect_w, int aspect_h)
{
    auto [w, h] = size;
    if (w * aspect_h > aspect_w * h)
    {
        // Image is too wide
        const int new_w = (aspect_w * h) / aspect_h;
        return cv::Rect((w - new_w) / 2, 0, new_w, h);
    }
    else
    {
        const int new_h = (aspect_h * w) / aspect_w;
        return cv::Rect(0, (h - new_h) / 2, w, new_h);
    }
}

void Preview::init(const cv_video_widget& widget)
{
    auto [w, h] = widget.preview_size();
    preview_size_ = { w, h };
}

void Preview::copy_video_frame(const cv::Mat& frame)
{
    cv::Rect roi = make_crop_rect_for_aspect(frame.size(), preview_size_.width, preview_size_.height);

    cv::resize(frame(roi), preview_image_, preview_size_, 0, 0, cv::INTER_NEAREST);

    offset_ = { (float)-roi.x, (float)-roi.y };
    scale_ = float(preview_image_.cols) / float(roi.width);
}

void Preview::draw_gizmos(const std::optional<PoseEstimator::Face>& face,
                          const std::optional<cv::Rect2f>& last_roi,
                          const std::optional<cv::Rect2f>& last_localizer_roi,
                          const cv::Point2f& neckjoint_position)
{
    if (preview_image_.empty())
        return;

    if (last_roi)
    {
        const int col = 255;
        cv::rectangle(preview_image_, transform(*last_roi), cv::Scalar(0, col, 0), /*thickness=*/1);
    }
    if (last_localizer_roi)
    {
        const int col = 255;
        cv::rectangle(preview_image_, transform(*last_localizer_roi), cv::Scalar(col, 0, 255 - col), /*thickness=*/1);
    }

    if (face)
    {
        if (face->size >= 1.f)
            cv::circle(preview_image_, static_cast<cv::Point>(transform(face->center)), int(transform(face->size)), cv::Scalar(255, 255, 255), 2);
        cv::circle(preview_image_, static_cast<cv::Point>(transform(face->center)), 3, cv::Scalar(255, 255, 255), -1);

        const cv::Matx33f R = face->rotation.toRotMat3x3(cv::QUAT_ASSUME_UNIT);

        auto draw_coord_line = [&](int i, const cv::Scalar& color)
        {
            const float vx = R(0, i);
            const float vy = R(1, i);
            static constexpr float len = 100.f;
            cv::Point q = face->center + len * cv::Point2f{ vx, vy };
            cv::line(preview_image_, static_cast<cv::Point>(transform(face->center)), static_cast<cv::Point>(transform(q)), color, 2);
        };
        draw_coord_line(0, { 0, 0, 255 });
        draw_coord_line(1, { 0, 255, 0 });
        draw_coord_line(2, { 255, 0, 0 });

        // Draw the computed joint position
        auto xy = transform(neckjoint_position);
        cv::circle(preview_image_, cv::Point(xy.x, xy.y), 5, cv::Scalar(0, 0, 255), -1);
    }
}

void Preview::overlay_netinput(const cv::Mat& netinput)
{
    if (netinput.empty())
        return;

    const int w = std::min(netinput.cols, preview_image_.cols);
    const int h = std::min(netinput.rows, preview_image_.rows);
    cv::Rect roi(0, 0, w, h);
    netinput(roi).copyTo(preview_image_(roi));
}

void Preview::draw_fps(double fps, double last_inference_time)
{
    char buf[128];
    ::snprintf(buf, sizeof(buf), "%d Hz, pose inference: %d ms", std::clamp(int(fps), 0, 9999), int(last_inference_time));
    cv::putText(preview_image_, buf, cv::Point(10, preview_image_.rows - 10), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 255, 0), 1);
}

void Preview::copy_to_widget(cv_video_widget& widget)
{
    if (preview_image_.rows > 0)
        widget.update_image(preview_image_);
}

cv::Rect2f Preview::transform(const cv::Rect2f& r) const
{
    return { (r.x - offset_.x) * scale_, (r.y - offset_.y) * scale_, r.width * scale_, r.height * scale_ };
}

cv::Point2f Preview::transform(const cv::Point2f& p) const
{
    return { (p.x - offset_.x) * scale_, (p.y - offset_.y) * scale_ };
}

float Preview::transform(float s) const
{
    return s * scale_;
}

} // namespace neuralnet_tracker_ns