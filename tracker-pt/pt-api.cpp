#include "pt-api.hpp"
#include "cv/numeric.hpp"

using namespace numeric_types;

pt_camera_info::pt_camera_info() = default;

f pt_camera_info::get_focal_length(f fov, int res_x, int res_y)
{
    const f diag_len = std::sqrt(f(res_x*res_x + res_y*res_y));
    const f aspect_x = res_x / diag_len;
    //const double aspect_y = res_y / diag_len;
    const f diag_fov = fov * pi/180;
    const f fov_x = 2*std::atan(std::tan(diag_fov*f{.5}) * aspect_x);
    //const double fov_y = 2*atan(tan(diag_fov*.5) * aspect_y);
    const f fx = f{.5} / std::tan(fov_x * f{.5});
    return fx;
    //fy = .5 / tan(fov_y * .5);
    //static bool once = false; if (!once) { once = true; qDebug() << "f" << ret << "fov" << (fov * 180/M_PI); }
}

pt_camera::pt_camera() = default;
pt_camera::~pt_camera() = default;
pt_runtime_traits::pt_runtime_traits() = default;
pt_runtime_traits::~pt_runtime_traits() = default;
pt_point_extractor::pt_point_extractor() = default;
pt_point_extractor::~pt_point_extractor() = default;

f pt_point_extractor::threshold_radius_value(int w, int h, int threshold)
{
    f cx = w / f{640}, cy = h / f{480};

    const f min_radius = f{1.75} * cx;
    const f max_radius = f{30} * cy;

    const f radius = std::fmax(f{0}, (max_radius-min_radius) * threshold / f(255) + min_radius);

    return radius;
}

std::tuple<f, f> pt_pixel_pos_mixin::to_pixel_pos(f x, f y, int w, int h)
{
    return { w*(x+f{.5}), f{.5}*(h - 2*y*w) };
}

std::tuple<f, f> pt_pixel_pos_mixin::to_screen_pos(f px, f py, int w, int h)
{
    px *= w/(w-f{1}); py *= h/(h-f{1});
    return { (px - w/f{2})/w, -(py - h/f{2})/w };
}

pt_frame::pt_frame() = default;
pt_frame::~pt_frame() = default;
