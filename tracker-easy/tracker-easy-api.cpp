#include "tracker-easy-api.h"
#include "cv/numeric.hpp"

using namespace numeric_types;



pt_runtime_traits::pt_runtime_traits() = default;
pt_runtime_traits::~pt_runtime_traits() = default;
pt_point_extractor::pt_point_extractor() = default;
pt_point_extractor::~pt_point_extractor() = default;

f pt_point_extractor::threshold_radius_value(int w, int h, int threshold)
{
    f cx = w / f{640}, cy = h / f{480};

    const f min_radius = f{1.75} * cx;
    const f max_radius = f{15} * cy;

    const f radius = std::fmax(f{0}, (max_radius-min_radius) * threshold / f(255) + min_radius);

    return radius;
}

std::tuple<f, f> pt_pixel_pos_mixin::to_pixel_pos(f x, f y, int w, int h)
{
    return std::make_tuple(w*(x+f{.5}), f{.5}*(h - 2*y*w));
}

std::tuple<f, f> pt_pixel_pos_mixin::to_screen_pos(f px, f py, int w, int h)
{
    px *= w/(w-f{1}); py *= h/(h-f{1});
    return std::make_tuple((px - w/f{2})/w, -(py - h/f{2})/w);
}

