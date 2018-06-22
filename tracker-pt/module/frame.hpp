#pragma once

#include "pt-api.hpp"

#include <opencv2/core.hpp>
#include <QImage>

namespace pt_module {

struct Frame final : pt_frame
{
    cv::Mat mat;

    operator const cv::Mat&() const& { return mat; }
    operator cv::Mat&() & { return mat; }
};

struct Preview final : pt_preview
{
    Preview(int w, int h);

    Preview& operator=(const pt_frame& frame) override;
    QImage get_bitmap() override;
    void draw_head_center(double x, double y) override;

    operator cv::Mat&() { return frame_copy; }
    operator cv::Mat const&() const { return frame_copy; }

private:
    static void ensure_size(cv::Mat& frame, int w, int h, int type);

    bool fresh = true;
    cv::Mat frame_copy, frame_color, frame_out, frame_out2;
};

} // ns pt_module
