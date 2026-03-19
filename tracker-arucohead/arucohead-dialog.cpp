/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "arucohead-tracker.h"
#include "arucohead-dialog.h"
#include "arucohead-help-dialog.h"
#include "api/plugin-api.hpp"
#include <opencv2/objdetect.hpp>
#include <QPushButton>

arucohead_dialog::arucohead_dialog() : // NOLINT(cppcoreguidelines-pro-type-member-init)
    tracker(nullptr)
{
    ui.setupUi(this);

    for (const auto& str : video::camera_names())
        ui.cmbCameraName->addItem(str, str);

    ui.cmbArucoDictionary->addItem("Original ArUco", arucohead_dictionary::ARUCOHEAD_DICT_ARUCO_ORIGINAL);
    ui.cmbArucoDictionary->addItem("ArUco MIP 36h12", arucohead_dictionary::ARUCOHEAD_DICT_ARUCO_MIP_36h12);
    ui.cmbArucoDictionary->addItem("AprilTag 36h11", arucohead_dictionary::ARUCOHEAD_DICT_APRILTAG_36h11);

    tie_setting(s.frame_width, ui.sbFrameWidth);
    tie_setting(s.frame_height, ui.sbFrameHeight);
    tie_setting(s.fps, ui.sbFPS);
    tie_setting(s.use_mjpeg, ui.cbUseMJPEG);
    tie_setting(s.aruco_marker_size_mm, ui.sbMarkerSize);
    tie_setting(s.number_of_markers, ui.sbNumerOfMarkers);
    tie_setting(s.first_marker_id, ui.sbFirstMarkerID);
    tie_setting(s.head_circumference_cm, ui.sbHeadCircumference);
    tie_setting(s.shoulder_to_marker_cm, ui.sbShoulderToMarkerDistance);
    tie_setting(s.aruco_dictionary, ui.cmbArucoDictionary);
    tie_setting(s.camera_name, ui.cmbCameraName);
    tie_setting(s.zoom, ui.sbZoom);
    tie_setting(s.fov, ui.sbFOV);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    connect(ui.btnHelp, SIGNAL(clicked()), this, SLOT(doShowHelp()));
    connect(ui.btnCameraSettings, SIGNAL(clicked()), this, SLOT(doOpenCameraSettings()));
}

void arucohead_dialog::register_tracker(ITracker* x)
{
    tracker = static_cast<arucohead_tracker*>(x);
}

void arucohead_dialog::unregister_tracker()
{
    tracker = nullptr;
}

void arucohead_dialog::set_buttons_visible(bool x)
{
    ui.buttonBox->setVisible(x);
}

void arucohead_dialog::doOK()
{
    save();
    close();
}

void arucohead_dialog::doCancel()
{
    reload();
    close();
}

void arucohead_dialog::doOpenCameraSettings()
{
    if (tracker) {
        QMutexLocker l(&tracker->camera_mtx);
        (void)tracker->camera->show_dialog();
    }
    else {
        (void)video::show_dialog(s.camera_name);
    }
}

void arucohead_dialog::doShowHelp()
{
    ArucoheadHelpDialog helpDlg(this);
    helpDlg.exec();
}

void arucohead_dialog::save()
{
    s.b->save();
}

void arucohead_dialog::reload()
{
    s.b->reload();
}
