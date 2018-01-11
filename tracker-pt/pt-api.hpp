#pragma once

#include "export.hpp"

#include "cv/numeric.hpp"
#include "options/options.hpp"

#include <memory>

#include <opencv2/core.hpp>

struct OTR_PT_EXPORT pt_camera_info final
{
    pt_camera_info();
    virtual double get_focal_length() const;

    double fov = 0;
    double fps = 0;

    int res_x = 0;
    int res_y = 0;
    int idx = -1;
};

enum pt_camera_open_status : unsigned { cam_open_error, cam_open_ok_no_change, cam_open_ok_change };

enum pt_color_type
{
    // explicit values, gotta preserve the numbering in .ini
    // don't reuse when removing some of the modes
    pt_color_natural = 2,
    pt_color_red_only = 3,
    pt_color_average = 5,
    pt_color_blue_only = 6,
};

struct OTR_PT_EXPORT pt_camera
{
    using result = std::tuple<bool, pt_camera_info>;

    pt_camera();
    virtual ~pt_camera();

    virtual warn_result_unused pt_camera_open_status start(int idx, int fps, int res_x, int res_y) = 0;
    virtual void stop() = 0;
    virtual warn_result_unused result get_frame(cv::Mat& frame) = 0;

    virtual warn_result_unused result get_info() const = 0;
    virtual pt_camera_info get_desired() const = 0;

    virtual QString get_desired_name() const = 0;
    virtual QString get_active_name() const = 0;

    virtual void set_fov(double value) = 0;
    virtual operator bool() const = 0;

    virtual void show_camera_settings() = 0;
};

struct OTR_PT_EXPORT pt_point_extractor
{
    using vec2 = types::vec2;

    pt_point_extractor();
    virtual ~pt_point_extractor();
    virtual void extract_points(const cv::Mat& frame, cv::Mat& preview_frame, std::vector<vec2>& points) = 0;

    static double threshold_radius_value(int w, int h, int threshold);
};

struct OTR_PT_EXPORT pt_settings final : options::opts
{
    using slider_value = options::slider_value;

    pt_settings(const QString& name);

    value<QString> camera_name { b, "camera-name", "" };
    value<int> cam_res_x { b, "camera-res-width", 640 },
               cam_res_y { b, "camera-res-height", 480 },
               cam_fps { b, "camera-fps", 30 };
    value<double> min_point_size { b, "min-point-size", 2.5 },
                  max_point_size { b, "max-point-size", 50 };

    value<int> m01_x { b, "m_01-x", 0 }, m01_y { b, "m_01-y", 0 }, m01_z { b, "m_01-z", 0 };
    value<int> m02_x { b, "m_02-x", 0 }, m02_y { b, "m_02-y", 0 }, m02_z { b, "m_02-z", 0 };

    value<int> t_MH_x { b, "model-centroid-x", 0 },
               t_MH_y { b, "model-centroid-y", 0 },
               t_MH_z { b, "model-centroid-z", 0 };

    value<int> clip_ty { b, "clip-ty", 40 },
               clip_tz { b, "clip-tz", 30 },
               clip_by { b, "clip-by", 70 },
               clip_bz { b, "clip-bz", 80 };

    value<int> active_model_panel { b, "active-model-panel", 0 },
               cap_x { b, "cap-x", 40 },
               cap_y { b, "cap-y", 60 },
               cap_z { b, "cap-z", 100 };

    value<int> fov { b, "camera-fov", 56 };

    value<bool> dynamic_pose { b, "dynamic-pose-resolution", true };
    value<int> init_phase_timeout { b, "init-phase-timeout", 250 };
    value<bool> auto_threshold { b, "automatic-threshold", true };
    value<pt_color_type> blob_color { b, "blob-color", pt_color_natural };

    value<slider_value> threshold_slider { b, "threshold-slider", slider_value(128, 0, 255) };
};

struct OTR_PT_EXPORT pt_runtime_traits
{
    pt_runtime_traits();
    virtual ~pt_runtime_traits();

    virtual std::unique_ptr<pt_camera> make_camera() const = 0;
    virtual std::unique_ptr<pt_point_extractor> make_point_extractor() const = 0;
    virtual QString get_module_name() const = 0;
};
