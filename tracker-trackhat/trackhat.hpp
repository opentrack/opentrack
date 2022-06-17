#pragma once

#include "../tracker-pt/pt-api.hpp"
#include "compat/macros.hpp"
#include "compat/timer.hpp"
#include "options/options.hpp"

#include <track_hat_driver.h>

#include <array>
#include <atomic>
#include <optional>
#include <opencv2/core/mat.hpp>

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

TH_ErrorCode log_error(TH_ErrorCode error, const char* source, const char* file, int line, const char* function);
#define th_check_(expr, expr2) ::trackhat_impl::log_error((expr), expr2)
#define th_check(expr) ::trackhat_impl::log_error((expr), #expr, __FILE__, __LINE__, function_name)

enum class led_mode : unsigned char {
    off, constant, dynamic,
};

struct trackhat_settings : opts
{
    static constexpr int min_gain = 1, max_gain = 47,
                         min_exposure = 0x10, max_exposure = 0xff;
    static constexpr int num_exposure_steps = max_gain + max_exposure - min_gain - min_exposure;
    trackhat_settings();
    value<slider_value> exposure{b, "exposure", {min_exposure + max_exposure*2/3, min_exposure, max_exposure}};
    value<slider_value> gain{b, "gain", {min_gain + max_gain*2/3, min_gain, max_gain}};
    //value<slider_value> threshold{b, "threshold", {0x97, 64, 0xfe}};
    value<model_type> model{b, "model", model_mini_clip_left};
    value<double> min_pt_size{b, "min-point-size", 10};
    value<double> max_pt_size{b, "max-point-size", 50};
    value<bool> enable_point_filter{b, "enable-point-filter", true };
    value<slider_value> point_filter_coefficient{b, "point-filter-coefficient", { 1.5, 1, 4 }};
    value<slider_value> point_filter_limit { b, "point-filter-limit", { 0.1, 0.01, 1 }};
    value<slider_value> point_filter_deadzone { b, "point-filter-deadzone", {0, 0, 1}};
    value<led_mode> led { b, "led-mode", led_mode::dynamic };
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

enum class led_state : unsigned char {
    invalid, stopped, not_tracking, tracking,
};

struct led_updater final {
    trackHat_SetLeds_t leds_ {TH_UNCHANGED, TH_UNCHANGED, TH_UNCHANGED};
    std::optional<Timer> timer_;
    led_state state_ = led_state::invalid;

    trackHat_SetLeds_t next_state(led_mode mode, led_state new_state);
    void update_(trackHat_Device_t* device, trackHat_SetLeds_t leds);
    void update(trackHat_Device_t* device, led_mode mode, led_state new_state);

    static constexpr int SWITCH_TIME_MS = 2000;
    static constexpr
        trackHat_SetLeds_t LED_idle = {TH_OFF, TH_SOLID, TH_OFF},
                           LED_off  = {TH_OFF, TH_OFF, TH_OFF},
                           LED_tracking {TH_OFF, TH_SOLID, TH_SOLID},
                           LED_not_tracking {TH_SOLID, TH_OFF, TH_OFF};
};

} // ns trackhat_impl

using trackhat_impl::trackhat_settings;
using trackhat_impl::led_updater;

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

struct camera_handle final
{
    OTR_DISABLE_MOVE_COPY(camera_handle);
    trackHat_Device_t* operator->() { return &device_; }
    trackHat_Device_t& operator*() { return device_; }

    camera_handle() = default;
    ~camera_handle() = default;

    constexpr operator bool() const { return state_ >= st_streaming; }
    [[nodiscard]] bool ensure_connected();
    [[nodiscard]] bool ensure_device_exists();
    void disconnect();
private:
    trackHat_Device_t device_ = {};
    enum state { st_stopped, st_detected, st_streaming, };
    state state_ = st_stopped;
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

    f deadzone_amount() const override { return 10; }

    static constexpr int sensor_size = 4096;
    static constexpr int sensor_fov = 52;
    static constexpr int point_count = TRACK_HAT_NUMBER_OF_POINTS;
    static constexpr bool debug_mode = true;

private:
    trackhat_impl::setting_receiver sig{true};

    [[nodiscard]] bool init_regs();
    void set_pt_options();

    camera_handle device;
    pt_settings s{trackhat_metadata::module_name};
    trackhat_settings t;
    led_updater led;
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
