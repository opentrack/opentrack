#pragma once

#include "pt-settings.hpp"

#include "cv/numeric.hpp"
#include "options/options.hpp"
#include "video/camera.hpp"

#include <tuple>
#include <type_traits>
#include <memory>

#include <opencv2/core.hpp>

#include <QImage>
#include <QString>

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wweak-vtables"
#endif

const int KPointCount = 3;

///
/// Utility class providing coordinates conversion functionalities
///
struct Coordinates final
{
    using f = numeric_types::f;

    static std::tuple<f, f> to_pixel_pos(f x, f y, int w, int h);
    static std::tuple<f, f> to_screen_pos(f px, f py, int w, int h);
};


struct pt_point_extractor
{
    using vec2 = numeric_types::vec2;
    using f = numeric_types::f;

    pt_point_extractor();
    virtual ~pt_point_extractor();
    virtual void extract_points(const cv::Mat& image, cv::Mat& preview_frame, std::vector<vec2>& points, std::vector<vec2>& imagePoints) = 0;

    static f threshold_radius_value(int w, int h, int threshold);
};

struct pt_runtime_traits
{
    template<typename t> using pointer = std::shared_ptr<t>;

    pt_runtime_traits();
    virtual ~pt_runtime_traits();

    virtual pointer<pt_point_extractor> make_point_extractor() const = 0;
    virtual QString get_module_name() const = 0;
};

template<typename t>
using pt_pointer = typename pt_runtime_traits::pointer<t>;

#ifdef __clang__
#   pragma clang diagnostic pop
#endif
