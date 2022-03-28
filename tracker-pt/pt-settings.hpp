#pragma once

#include "options/options.hpp"

#include <QString>

enum pt_color_type
{
    // explicit values, gotta preserve the numbering in .ini
    // don't reuse when removing some of the modes
    pt_color_bt709 = 2,
    pt_color_hardware = 14,
    pt_color_red_only = 3,
    pt_color_blue_only = 6,
    pt_color_green_only = 7,
    pt_color_red_chromakey = 8,
    pt_color_green_chromakey = 9,
    pt_color_blue_chromakey = 10,
    pt_color_cyan_chromakey = 11,
    pt_color_yellow_chromakey = 12,
    pt_color_magenta_chromakey = 13,
};

namespace pt_impl {

using namespace options;

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wweak-vtables"
#endif

struct pt_settings final : options::opts
{
    using slider_value = options::slider_value;

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

    value<bool> dynamic_pose { b, "dynamic-pose-resolution", false };
    value<int> init_phase_timeout { b, "init-phase-timeout", 250 };
    value<bool> auto_threshold { b, "automatic-threshold", true };
    value<pt_color_type> blob_color { b, "blob-color", pt_color_bt709 };
    value<bool> use_mjpeg { b, "use-mjpeg", false };

    value<slider_value> threshold_slider { b, "threshold-slider", { 128, 0, 255 } };

    value<bool> enable_point_filter{ b, "enable-point-filter", false };
    value<slider_value> point_filter_coefficient { b, "point-filter-coefficient", { 1.0, 0, 4 } };
    value<slider_value> point_filter_limit { b, "point-filter-limit", { 0.1, 0.01, 1 }};
    value<slider_value> point_filter_deadzone { b, "point-filter-deadzone", {0, 0, 1} };

    explicit pt_settings(const QString& name) : opts(name) {}
};

#ifdef __clang__
#   pragma clang diagnostic pop
#endif

} // ns pt_impl

using pt_settings = pt_impl::pt_settings;
