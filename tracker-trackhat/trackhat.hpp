#pragma once

#include "../tracker-pt/pt-api.hpp"
#include "track_hat_driver.h"
#include "compat/macros.hpp"

#include <opencv2/core.hpp>

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

    static constexpr int sensor_size = 2940*2;
    static constexpr int sensor_fov = 52;
    static constexpr int point_count = TRACK_HAT_NUMBER_OF_POINTS;

private:
    enum device_status { th_noinit, th_init, th_detect, th_connect, th_running, };

    trackHat_Device_t device {};
    device_status status = th_noinit;
    TH_ErrorCode error_code = TH_SUCCESS;
};

struct trackhat_frame final : pt_frame
{
    trackHat_Points_t points = {};

    trackhat_frame() = default;
    ~trackhat_frame() override = default;
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
    trackHat_Points_t points = {};
};

struct trackhat_extractor final : pt_point_extractor
{
    void extract_points(const pt_frame& data, pt_preview&, bool, std::vector<vec2>& points) override;

    OTR_DISABLE_MOVE_COPY(trackhat_extractor);

    trackhat_extractor() = default;
    ~trackhat_extractor() override = default;
};

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
};
