/* Copyright (c) 2023 Eyeware Tech SA https://www.eyeware.tech
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "api/plugin-api.hpp"

#include "ui_eyeware_beam.h"

#include "eyeware/tracker_client.h"

#include <QObject>
#include <QMutex>

class eyeware_beam_tracker : public QObject, public ITracker
{
    Q_OBJECT

public:
    eyeware_beam_tracker();
    ~eyeware_beam_tracker() override;
    module_status start_tracker(QFrame* frame) override;
    void data(double *data) override;

private:
    void extract_translation(const eyeware::Vector3D& t,
                             double& translation_x_cm,
                             double& translation_y_cm,
                             double& translation_z_cm);
    void extract_rotation_angles(const eyeware::Matrix3x3& R, double& pitch_deg, double& roll_deg, double& yaw_deg);

    eyeware::TrackerClient* tracker_client = nullptr;

    QMutex mtx;

    double last_pitch_deg = 0.0;
    double last_roll_deg = 0.0;
    double last_yaw_deg = 0.0;
    double last_translation_x_cm = 0.0;
    double last_translation_y_cm = 0.0;
    double last_translation_z_cm = 0.0;
};

class eyeware_beam_dialog : public ITrackerDialog
{
    Q_OBJECT

public:
    eyeware_beam_dialog();
    void register_tracker(ITracker * x) override { tracker = static_cast<eyeware_beam_tracker*>(x); }
    void unregister_tracker() override { tracker = nullptr; }

private:
    Ui::eyeware_beam_ui ui;
    eyeware_beam_tracker* tracker = nullptr;

private Q_SLOTS:
    void doOK();
};

class eyeware_beam_metadata : public Metadata
{
    Q_OBJECT
    QString name() override { return QString("Eyeware Beam"); }
    QIcon icon() override { return QIcon(":/images/eyeware_beam_logo.png"); }
};
