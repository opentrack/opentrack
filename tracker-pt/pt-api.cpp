#include "pt-api.hpp"
#include "cv/numeric.hpp"

using namespace types;

pt_camera_info::pt_camera_info() = default;

double pt_camera_info::get_focal_length(f fov, int res_x, int res_y)
{
    const double diag_len = std::sqrt(double(res_x*res_x + res_y*res_y));
    const double aspect_x = res_x / diag_len;
    //const double aspect_y = res_y / diag_len;
    const double diag_fov = fov * M_PI/180;
    const double fov_x = 2*std::atan(std::tan(diag_fov*.5) * aspect_x);
    //const double fov_y = 2*atan(tan(diag_fov*.5) * aspect_y);
    const double fx = .5 / std::tan(fov_x * .5);
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

double pt_point_extractor::threshold_radius_value(int w, int h, int threshold)
{
    double cx = w / 640., cy = h / 480.;

    const double min_radius = 1.75 * cx;
    const double max_radius = 15 * cy;

    const double radius = std::fmax(0., (max_radius-min_radius) * threshold / f(255) + min_radius);

    return radius;
}

std::tuple<double, double> pt_pixel_pos_mixin::to_pixel_pos(double x, double y, int w, int h)
{
    return std::make_tuple(w*(x+.5), .5*(h - 2*y*w));
}

std::tuple<double, double> pt_pixel_pos_mixin::to_screen_pos(double px, double py, int w, int h)
{
    px *= w/(w-1.); py *= h/(h-1.);
    return std::make_tuple((px - w/2.)/w, -(py - h/2.)/w);
}

pt_frame::pt_frame() = default;

pt_frame::~pt_frame() = default;
