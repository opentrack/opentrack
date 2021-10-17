#pragma once

#include "../tracker-pt/pt-api.hpp"
#include "compat/macros.hpp"
#include "options/options.hpp"

#include <track_hat_driver.h>

#include <array>
#include <atomic>
#include <opencv2/core.hpp>

enum model_type : int
{
    model_cap = 1,
    model_clip_left,
    model_clip_right,
    model_mini_clip_left,
    model_mini_clip_right,
    model_mystery_meat,
};

namespace trackhat_impl
{
using namespace options;

struct trackhat_settings : opts
{
    trackhat_settings();
    value<slider_value> exposure{b, "exposure", {0x80, 0x10, 0xff}};
    value<slider_value> gain{b, "gain", {16, 0, 47}};
    value<slider_value> threshold{b, "threshold", {0x97, 64, 0xfe}};
    value<slider_value> threshold_2{b, "threshold-2", {0x03, 1, 0xfe}};
    value<model_type> model{b, "model", model_mini_clip_left};
    value<double> min_pt_size{b, "min-point-size", 10};
    value<double> max_pt_size{b, "max-point-size", 50};
    value<bool> enable_point_filter{b, "enable-point-filter", true };
    value<slider_value> point_filter_coefficient{b, "point-filter-coefficient", { 1.5, 1, 4 }};
};

class setting_receiver : public QObject
{
    Q_OBJECT

public:
    explicit setting_receiver(bool value);
    bool test_and_clear();
public slots:
    void settings_changed();
private:
    std::atomic<bool> changed{false};
};

} // ns trackhat_impl

using typename trackhat_impl::trackhat_settings;

struct trackhat_metadata final : pt_runtime_traits
{
    pt_runtime_traits::pointer<pt_camera> make_camera() const override;
    pt_runtime_traits::pointer<pt_point_extractor> make_point_extractor() const override;
    pt_runtime_traits::pointer<pt_frame> make_frame() const override;
    pt_runtime_traits::pointer<pt_preview> make_preview(int w, int h) const override;
    QString get_module_name() const override;

    OTR_DISABLE_MOVE_COPY(trackhat_metadata);

    trackhat_metadata() = default;
    ~trackhat_metadata() override = default;

    static const QString module_name;
};

struct point
{
    int brightness = 0, area, x, y, W, H;
    bool ok = false;
};

struct trackhat_camera final : pt_camera
{
    trackhat_camera();
    ~trackhat_camera() override;

    OTR_DISABLE_MOVE_COPY(trackhat_camera);

    bool start(const pt_settings& s) override;
    void stop() override;

    pt_camera::result get_frame(pt_frame& frame) override;
    pt_camera::result get_info() const override;
    pt_camera_info get_desired() const override;

    QString get_desired_name() const override;
    QString get_active_name() const override;

    void set_fov(f value) override;
    void show_camera_settings() override;

    static constexpr int sensor_size = 2940;
    static constexpr int sensor_fov = 52;
    static constexpr int point_count = TRACK_HAT_NUMBER_OF_POINTS;

private:
    enum device_status { th_noinit, th_init, th_detect, th_connect, th_running, };
    trackhat_impl::setting_receiver sig{true};

    [[nodiscard]] int init_regs();
    void set_pt_options();

    trackHat_Device_t device {};
    device_status status = th_noinit;
    TH_ErrorCode error_code = TH_SUCCESS;
    pt_settings s{trackhat_metadata::module_name};
    trackhat_settings t;
};

struct trackhat_frame final : pt_frame
{
    void init_points(const trackHat_ExtendedPoints_t& points, double min_size, double max_size);
    trackhat_frame() = default;
    ~trackhat_frame() override = default;

    std::array<point, trackhat_camera::point_count> points;
};

struct trackhat_preview final : pt_preview
{
    QImage get_bitmap() override;
    void draw_head_center(f x, f y) override;
    void set_last_frame(const pt_frame&) override; // NOLINT(misc-unconventional-assign-operator)

    trackhat_preview(int w, int h);
    ~trackhat_preview() override = default;
    void draw_points();
    void draw_center();

    OTR_DISABLE_MOVE_COPY(trackhat_preview);

    cv::Mat frame_bgr, frame_bgra;
    numeric_types::vec2 center{-1, -1};
    std::array<point, trackhat_camera::point_count> points;
};

struct trackhat_extractor final : pt_point_extractor
{
    void extract_points(const pt_frame& data, pt_preview&, bool, std::vector<vec2>& points) override;

    OTR_DISABLE_MOVE_COPY(trackhat_extractor);

    trackhat_extractor() = default;
    ~trackhat_extractor() override = default;
};
