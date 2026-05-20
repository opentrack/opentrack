/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once

#include "options/options.hpp"

using namespace options;

enum papertracker_dictionary {
    PAPERTRACKER_DICT_ARUCO_ORIGINAL,
    PAPERTRACKER_DICT_ARUCO_MIP_36h12,
    PAPERTRACKER_DICT_APRILTAG_36h11
};

struct papertracker_settings : public opts
{
    value<int> frame_width { b, "frame_width", 640 };
    value<int> frame_height { b, "frame_height", 480 };
    value<int> fps { b, "fps", 30 };
    value<bool> use_mjpeg { b, "use_mjpeg", false };
    value<int> aruco_marker_size_mm { b, "aruco_marker_size_mm", 80 };
    value<int> number_of_markers { b, "number_of_markers", 1 };
    value<int> first_marker_id { b, "first_marker_id", 1 };
    value<double> head_circumference_cm { b, "head_circumference_cm", 60 };
    value<double> marker_height_cm { b, "marker_height_cm", 15 };
    value<papertracker_dictionary> aruco_dictionary { b, "aruco_dictionary", papertracker_dictionary::PAPERTRACKER_DICT_ARUCO_MIP_36h12 };
    value<QString> camera_name { b, "camera_name", "" };
    value<int> zoom { b, "zoom", 100 };
    value<int> fov { b, "fov", 56 };
    value<int> marker_min_angle { b, "marker_min_angle", 15 };
    value<int> marker_max_angle { b, "marker_max_angle", 80 };

    papertracker_settings();
};

struct papertracker_static_settings
{
    int frame_width;
    int frame_height;
    int fps;
    bool use_mjpeg;
    QString camera_name;
    int aruco_marker_size_mm;
};
