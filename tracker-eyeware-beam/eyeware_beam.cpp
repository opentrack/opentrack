/* Copyright (c) 2023 Eyeware Tech SA https://www.eyeware.tech
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "eyeware_beam.h"

#include <QMutexLocker>

static constexpr double rad_to_deg = 180.0 / M_PI;
static constexpr double m_to_cm = 100.0;
static constexpr double epsilon = 0.000001;

eyeware_beam_tracker::eyeware_beam_tracker()
{
}

eyeware_beam_tracker::~eyeware_beam_tracker()
{
    QMutexLocker lck(&mtx);
    release_tracker_instance(tracker_client);
    tracker_client = nullptr;
}

module_status eyeware_beam_tracker::start_tracker(QFrame* videoframe)
{
    QMutexLocker lck(&mtx);
    try
    {
        tracker_client = create_tracker_instance("127.0.0.1", 12010);
    }
    catch (...)
    {
        return error("Eyeware Beam initialization has failed");
    }

    return status_ok();
}

void eyeware_beam_tracker::extract_translation(const eyeware::Vector3D& t,
                                               double& translation_x_cm,
                                               double& translation_y_cm,
                                               double& translation_z_cm)
{
    translation_x_cm = +t.x * m_to_cm;
    translation_y_cm = -t.y * m_to_cm;
    translation_z_cm = +t.z * m_to_cm;
}

void eyeware_beam_tracker::extract_rotation_angles(const eyeware::Matrix3x3& R,
                                                   double& pitch_deg,
                                                   double& roll_deg,
                                                   double& yaw_deg)
{
    double r00 = static_cast<double>(R[0][0]);
    double r01 = static_cast<double>(R[0][1]);
    double r02 = static_cast<double>(R[0][2]);
    double r10 = static_cast<double>(R[1][0]);
    double r11 = static_cast<double>(R[1][1]);
    double r12 = static_cast<double>(R[1][2]);
    double r20 = static_cast<double>(R[2][0]);
    double r21 = static_cast<double>(R[2][1]);
    double r22 = static_cast<double>(R[2][2]);

    double dy = std::sqrt(r00 * r00 + r10 * r10);
    last_yaw_deg = -std::atan2(-r20, dy) * rad_to_deg;
    last_roll_deg = 0.0;
    if (dy > epsilon)
    {
        last_pitch_deg = -std::atan2(r21, r22) * rad_to_deg;
        last_roll_deg = +std::atan2(r10, r00) * rad_to_deg;
    }
    else
    {
        last_pitch_deg = -std::atan2(-r12, r11) * rad_to_deg;
    }
}

void eyeware_beam_tracker::data(double *data)
{
     QMutexLocker lck(&mtx);

     if (connected(tracker_client))
     {
         eyeware::HeadPoseInfo head_pose_info = get_head_pose_info(tracker_client);
         if (!head_pose_info.is_lost)
         {
             extract_translation(head_pose_info.transform.translation, last_translation_x_cm,
                                 last_translation_y_cm, last_translation_z_cm);
             extract_rotation_angles(head_pose_info.transform.rotation, last_pitch_deg, last_roll_deg, last_yaw_deg);
         }
     }

     data[TX] = last_translation_x_cm;
     data[TY] = last_translation_y_cm;
     data[TZ] = last_translation_z_cm;
     data[Yaw] = last_yaw_deg;
     data[Pitch] = last_pitch_deg;
     data[Roll] = last_roll_deg;
}

eyeware_beam_dialog::eyeware_beam_dialog()
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
}

void eyeware_beam_dialog::doOK()
{
    close();
}

OPENTRACK_DECLARE_TRACKER(eyeware_beam_tracker, eyeware_beam_dialog, eyeware_beam_metadata)
