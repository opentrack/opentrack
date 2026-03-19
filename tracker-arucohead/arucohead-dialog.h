/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once

#include "ui_arucohead.h"
#include "api/plugin-api.hpp"
#include "options/options.hpp"

using namespace options;

enum arucohead_dictionary {
    ARUCOHEAD_DICT_ARUCO_ORIGINAL,
    ARUCOHEAD_DICT_ARUCO_MIP_36h12,
    ARUCOHEAD_DICT_APRILTAG_36h11
};

class arucohead_tracker;

struct settings : public opts
{
    value<int> frame_width { b, "frame_width", 640 };
    value<int> frame_height { b, "frame_height", 480 };
    value<int> fps { b, "fps", 30 };
    value<bool> use_mjpeg { b, "use_mjpeg", false };
    value<int> aruco_marker_size_mm { b, "aruco_marker_size_mm", 80 };
    value<int> number_of_markers { b, "number_of_markers", 1 };
    value<int> first_marker_id { b, "first_marker_id", 1 };
    value<double> head_circumference_cm { b, "head_circumference_cm", 60 };
    value<double> shoulder_to_marker_cm { b, "shoulder_to_marker_cm", 22 };
    value<arucohead_dictionary> aruco_dictionary { b, "aruco_dictionary", arucohead_dictionary::ARUCOHEAD_DICT_ARUCO_MIP_36h12 };
    value<QString> camera_name { b, "camera_name", "" };
    value<int> zoom { b, "zoom", 100 };
    value<int> fov { b, "fov", 60 };

    settings();
};

class arucohead_dialog : public ITrackerDialog
{
    Q_OBJECT

    Ui::arucohead_dialog ui;
    arucohead_tracker *tracker;

public:
    arucohead_dialog();
    void register_tracker(ITracker *) override;
    void unregister_tracker() override;
    bool embeddable() noexcept override { return true; }
    void set_buttons_visible(bool x) override;
    void save() override;
    void reload() override;
private:
    settings s;
private slots:
    void doOK();
    void doCancel();
    void doOpenCameraSettings();
    void doShowHelp();
};