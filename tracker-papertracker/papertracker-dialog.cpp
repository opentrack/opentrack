/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "papertracker.h"
#include "papertracker-dialog.h"
#include "papertracker-help-dialog.h"
#include "api/plugin-api.hpp"
#include <opencv2/objdetect.hpp>
#include <QPushButton>
#include <sstream>

PaperTrackerDialog::PaperTrackerDialog() : // NOLINT(cppcoreguidelines-pro-type-member-init)
    tracker(nullptr)
{
    ui.setupUi(this);

    for (const auto& str : video::camera_names())
        ui.cmbCameraName->addItem(str, str);

    ui.cmbArucoDictionary->addItem("Original ArUco", papertracker_dictionary::PAPERTRACKER_DICT_ARUCO_ORIGINAL);
    ui.cmbArucoDictionary->addItem("ArUco MIP 36h12", papertracker_dictionary::PAPERTRACKER_DICT_ARUCO_MIP_36h12);
    ui.cmbArucoDictionary->addItem("AprilTag 36h11", papertracker_dictionary::PAPERTRACKER_DICT_APRILTAG_36h11);

    ui.cmbArucoContrastMethod->addItem("Auto (Continuous)", papertracker_contrast_method::PAPERTRACKER_CONTRAST_METHOD_CONTINUOUS_AUTO);
    ui.cmbArucoContrastMethod->addItem("Auto (Locked)", papertracker_contrast_method::PAPERTRACKER_CONTRAST_METHOD_LOCKED_AUTO);
    ui.cmbArucoContrastMethod->addItem("Global", papertracker_contrast_method::PAPERTRACKER_CONTRAST_METHOD_GLOBAL);
    ui.cmbArucoContrastMethod->addItem("Adaptive (5 x 5)", papertracker_contrast_method::PAPERTRACKER_CONTRAST_METHOD_ADAPTIVE_5_5);
    ui.cmbArucoContrastMethod->addItem("Adaptive (7 x 7)", papertracker_contrast_method::PAPERTRACKER_CONTRAST_METHOD_ADAPTIVE_7_7);
    ui.cmbArucoContrastMethod->addItem("Adaptive (9 x 9)", papertracker_contrast_method::PAPERTRACKER_CONTRAST_METHOD_ADAPTIVE_9_9);
    ui.cmbArucoContrastMethod->addItem("Adaptive (11 x 11)", papertracker_contrast_method::PAPERTRACKER_CONTRAST_METHOD_ADAPTIVE_11_11);

    tie_setting(s.frame_width, ui.sbFrameWidth);
    tie_setting(s.frame_height, ui.sbFrameHeight);
    tie_setting(s.fps, ui.sbFPS);
    tie_setting(s.use_mjpeg, ui.cbUseMJPEG);
    tie_setting(s.aruco_marker_size_mm, ui.sbMarkerSize);
    tie_setting(s.number_of_markers, ui.sbNumberOfMarkers);
    tie_setting(s.first_marker_id, ui.sbFirstMarkerID);
    tie_setting(s.head_circumference_cm, ui.sbHeadCircumference);
    tie_setting(s.marker_height_cm, ui.sbMarkerHeight);
    tie_setting(s.aruco_dictionary, ui.cmbArucoDictionary);
    tie_setting(s.camera_name, ui.cmbCameraName);
    tie_setting(s.zoom, ui.sbZoom);
    tie_setting(s.fov, ui.sbFOV);
    tie_setting(s.marker_min_angle, ui.sbMarkerMinimumAngle);
    tie_setting(s.marker_max_angle, ui.sbMarkerMaximumAngle);
    tie_setting(s.contrast_method, ui.cmbArucoContrastMethod);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    connect(ui.btnHelp, SIGNAL(clicked()), this, SLOT(doShowHelp()));
    connect(ui.btnCameraSettings, SIGNAL(clicked()), this, SLOT(doOpenCameraSettings()));
    connect(&timer, &QTimer::timeout, this, &PaperTrackerDialog::doUpdateStatus);

    timer.setInterval(250);
    doUpdateStatus();
}

void PaperTrackerDialog::register_tracker(ITracker* x)
{
    tracker = static_cast<PaperTracker*>(x);
    doUpdateStatus();
    timer.start();
}

void PaperTrackerDialog::unregister_tracker()
{
    tracker = nullptr;
    doUpdateStatus();
    timer.stop();
}

void PaperTrackerDialog::set_buttons_visible(bool x)
{
    ui.buttonBox->setVisible(x);
}

void PaperTrackerDialog::doOK()
{
    save();
    close();
}

void PaperTrackerDialog::doCancel()
{
    reload();
    close();
}

void PaperTrackerDialog::doOpenCameraSettings()
{
    if (tracker) {
        QMutexLocker l(&tracker->camera_mtx);
        (void)tracker->camera->show_dialog();
    }
    else {
        (void)video::show_dialog(s.camera_name);
    }
}

void PaperTrackerDialog::doShowHelp()
{
    PaperTrackerHelpDialog helpDlg(this);
    helpDlg.exec();
}

void PaperTrackerDialog::save()
{
    s.b->save();
}

void PaperTrackerDialog::reload()
{
    s.b->reload();
}

void PaperTrackerDialog::doUpdateStatus()
{
    if (tracker != nullptr)
        tracker->update_settings();

    if (tracker == nullptr)
        setStatusLabel(tracker_status::STOPPED);
    else if (!tracker->tracking_started())
        setStatusLabel(tracker_status::STARTING);
    else if (tracker->restart_required())
        setStatusLabel(tracker_status::RESTART_REQUIRED);
    else
        setStatusLabel(tracker_status::RUNNING);
}

void PaperTrackerDialog::setStatusLabel(tracker_status status)
{
    std::stringstream ss;

    ss << "Tracker status: ";

    switch (status) {
        case tracker_status::STOPPED:
            ss << "Not started";
            break;
        case tracker_status::STARTING:
            ss << "Starting";
            break;
        case tracker_status::RUNNING:
            ss << "Running";
            break;
        case tracker_status::RESTART_REQUIRED:
            ss << "Restart required for some changes to take effect";
            break;
    }

    ui.lblStatus->setText(ss.str().c_str());
}
