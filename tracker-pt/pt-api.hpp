#pragma once

#include "pt-settings.hpp"

#include "cv/numeric.hpp"
#include "options/options.hpp"

#include <tuple>
#include <type_traits>
#include <memory>

#include <opencv2/core.hpp>

#include <QImage>

struct pt_camera_info final
{
    typedef typename types::f f;

    pt_camera_info();
    static double get_focal_length(f fov, int res_x, int res_y);

    double fov = 0;
    double fps = 0;

    int res_x = 0;
    int res_y = 0;
    int idx = -1;
};

struct pt_pixel_pos_mixin
{
    static std::tuple<double, double> to_pixel_pos(double x, double y, int w, int h);
    static std::tuple<double, double> to_screen_pos(double px, double py, int w, int h);
};

struct pt_frame : pt_pixel_pos_mixin
{
    pt_frame();
    virtual ~pt_frame();

    template<typename t>
    t* as() &
    {
        using u = std::decay_t<t>;
        static_assert(std::is_convertible_v<u*, pt_frame*>, "must be derived from pt_image");

        return static_cast<t*>(this);
    }

    template<typename t>
    t const* as_const() const&
    {
        return const_cast<pt_frame*>(this)->as<const t>();
    }
};

struct pt_preview : pt_frame
{
    virtual pt_preview& operator=(const pt_frame&) = 0;
    virtual QImage get_bitmap() = 0;
    virtual void draw_head_center(double x, double y) = 0;
};

struct pt_camera
{
    using result = std::tuple<bool, pt_camera_info>;

    pt_camera();
    virtual ~pt_camera();

    virtual warn_result_unused bool start(int idx, int fps, int res_x, int res_y) = 0;
    virtual void stop() = 0;
    virtual warn_result_unused result get_frame(pt_frame& frame) = 0;

    virtual warn_result_unused result get_info() const = 0;
    virtual pt_camera_info get_desired() const = 0;

    virtual QString get_desired_name() const = 0;
    virtual QString get_active_name() const = 0;

    virtual void set_fov(double value) = 0;
    virtual void show_camera_settings() = 0;
};

struct pt_point_extractor : pt_pixel_pos_mixin
{
    using vec2 = types::vec2;

    pt_point_extractor();
    virtual ~pt_point_extractor();
    virtual void extract_points(const pt_frame& image, pt_preview& preview_frame, std::vector<vec2>& points) = 0;

    static double threshold_radius_value(int w, int h, int threshold);
};

struct pt_runtime_traits
{
    template<typename t> using pointer = std::shared_ptr<t>;

    pt_runtime_traits();
    virtual ~pt_runtime_traits();

    virtual pointer<pt_camera> make_camera() const = 0;
    virtual pointer<pt_point_extractor> make_point_extractor() const = 0;
    virtual pointer<pt_frame> make_frame() const = 0;
    virtual pointer<pt_preview> make_preview(int w, int h) const = 0;
    virtual QString get_module_name() const = 0;
};
