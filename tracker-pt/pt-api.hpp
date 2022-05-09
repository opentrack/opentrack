#pragma once

#include "pt-settings.hpp"

#include "cv/numeric.hpp"
#include "options/options.hpp"

#include <tuple>
#include <vector>
#include <memory>

#include <QImage>
#include <QString>

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wweak-vtables"
#endif

struct pt_camera_info final
{
    using f = numeric_types::f;

    pt_camera_info();
    static f get_focal_length(f fov, int res_x, int res_y);

    f fov = 0;
    f fps = 0;

    int res_x = 0;
    int res_y = 0;
    QString name;
    bool use_mjpeg = false;
};

struct pt_pixel_pos_mixin
{
    using f = numeric_types::f;

    static std::tuple<f, f> to_pixel_pos(f x, f y, int w, int h);
    static std::tuple<f, f> to_screen_pos(f px, f py, int w, int h);
};

struct pt_frame : pt_pixel_pos_mixin
{
    pt_frame();
    virtual ~pt_frame();

    template<typename t>
    t* as() &
    {
        return static_cast<t*>(this);
    }

    template<typename t>
    t const* as_const() const&
    {
        return static_cast<t const*>(this);
    }

protected:
    pt_frame(const pt_frame&) = default;
    pt_frame(pt_frame&&) = default;
    pt_frame& operator=(const pt_frame&) = default;
    pt_frame& operator=(pt_frame&&) = default;
};

struct pt_preview : pt_frame
{
    pt_preview() = default;

    OTR_DISABLE_MOVE_COPY(pt_preview);

    virtual void set_last_frame(const pt_frame&) = 0;
    virtual QImage get_bitmap() = 0;
    virtual void draw_head_center(f x, f y) = 0;
};

struct pt_camera
{
    using result = std::tuple<bool, pt_camera_info>;
    using f = numeric_types::f;

    pt_camera();
    virtual ~pt_camera();

    OTR_DISABLE_MOVE_COPY(pt_camera);

    [[nodiscard]] virtual bool start(const pt_settings& s) = 0;
    virtual void stop() = 0;

    virtual result get_frame(pt_frame& frame) = 0;
    virtual result get_info() const = 0;
    virtual pt_camera_info get_desired() const = 0;

    virtual QString get_desired_name() const = 0;
    virtual QString get_active_name() const = 0;

    virtual void set_fov(f value) = 0;
    virtual void show_camera_settings() = 0;
    virtual f deadzone_amount() const { return 1; }
};

struct pt_point_extractor : pt_pixel_pos_mixin
{
    using vec2 = numeric_types::vec2;
    using f = numeric_types::f;

    OTR_DISABLE_MOVE_COPY(pt_point_extractor);

    pt_point_extractor();
    virtual ~pt_point_extractor();
    virtual void extract_points(const pt_frame& image, pt_preview& preview_frame, bool preview_visible, std::vector<vec2>& points) = 0;

    static f threshold_radius_value(int w, int h, int threshold);
};

struct pt_runtime_traits
{
    template<typename t> using pointer = std::shared_ptr<t>;

    OTR_DISABLE_MOVE_COPY(pt_runtime_traits);

    pt_runtime_traits();
    virtual ~pt_runtime_traits();

    virtual pointer<pt_camera> make_camera() const = 0;
    virtual pointer<pt_point_extractor> make_point_extractor() const = 0;
    virtual pointer<pt_frame> make_frame() const = 0;
    virtual pointer<pt_preview> make_preview(int w, int h) const = 0;
    virtual QString get_module_name() const = 0;
};

template<typename t>
using pt_pointer = typename pt_runtime_traits::pointer<t>;

#ifdef __clang__
#   pragma clang diagnostic pop
#endif
