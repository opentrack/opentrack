/* Copyright (c) 2023, Khoa Nguyen <khoanguyen@3forcom.com>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "tobii.h"
#include "compat/math-imports.hpp"

#include <QMutexLocker>

static constexpr double rad_to_deg = 180.0 * M_1_PI;
static constexpr double mm_to_cm = 0.1;

static void url_receiver(char const* url, void* user_data)
{
    char* buffer = (char*)user_data;
    if (*buffer != '\0')
        return; // only keep first value

    if (strlen(url) < 256)
        strcpy(buffer, url);
}

static void head_pose_callback(tobii_head_pose_t const* head_pose, void* user_data)
{
    // Store the latest head pose data in the supplied storage
    tobii_head_pose_t* head_pose_storage = (tobii_head_pose_t*)user_data;
    *head_pose_storage = *head_pose;
}

tobii_tracker::tobii_tracker() = default;

tobii_tracker::~tobii_tracker()
{
    QMutexLocker lck(&mtx);
    if (device)
    {
        tobii_head_pose_unsubscribe(device);
        tobii_device_destroy(device);
    }
    if (api)
    {
        tobii_api_destroy(api);
    }
}

module_status tobii_tracker::start_tracker(QFrame*)
{
    QMutexLocker lck(&mtx);
    tobii_error_t tobii_error = tobii_api_create(&api, nullptr, nullptr);
    if (tobii_error != TOBII_ERROR_NO_ERROR)
    {
        return error("Failed to initialize the Tobii Stream Engine API.");
    }

    char url[256] = { 0 };
    tobii_error = tobii_enumerate_local_device_urls(api, url_receiver, url);
    if (tobii_error != TOBII_ERROR_NO_ERROR || url[0] == '\0')
    {
        tobii_api_destroy(api);
        return error("No stream engine compatible device(s) found.");
    }

    tobii_error = tobii_device_create(api, url, &device);
    if (tobii_error != TOBII_ERROR_NO_ERROR)
    {
        tobii_api_destroy(api);
        return error(QString("Failed to connect to %1.").arg(url));
    }

    tobii_error = tobii_head_pose_subscribe(device, head_pose_callback, &latest_head_pose);
    if (tobii_error != TOBII_ERROR_NO_ERROR)
    {
        tobii_device_destroy(device);
        tobii_api_destroy(api);
        return error("Failed to subscribe to head pose stream.");
    }

    return status_ok();
}

void tobii_tracker::data(double* data)
{
    QMutexLocker lck(&mtx);
    tobii_error_t tobii_error = tobii_device_process_callbacks(device);
    if (tobii_error != TOBII_ERROR_NO_ERROR)
    {
        return;
    }

    // Tobii coordinate system is different from opentrack's
    // Tobii: +x is to the right, +y is up, +z is towards the user
    // Rotation xyz is in radians, x is pitch, y is yaw, z is roll

    if (latest_head_pose.position_validity == TOBII_VALIDITY_VALID)
    {
        data[TX] = -latest_head_pose.position_xyz[0] * mm_to_cm;
        data[TY] = latest_head_pose.position_xyz[1] * mm_to_cm;
        data[TZ] = latest_head_pose.position_xyz[2] * mm_to_cm;
    }

    if (latest_head_pose.rotation_validity_xyz[0] == TOBII_VALIDITY_VALID)
    {
        data[Pitch] = latest_head_pose.rotation_xyz[0] * rad_to_deg;
    }

    if (latest_head_pose.rotation_validity_xyz[1] == TOBII_VALIDITY_VALID)
    {
        data[Yaw] = -latest_head_pose.rotation_xyz[1] * rad_to_deg;
    }

    if (latest_head_pose.rotation_validity_xyz[2] == TOBII_VALIDITY_VALID)
    {
        data[Roll] = latest_head_pose.rotation_xyz[2] * rad_to_deg;
    }
}

OPENTRACK_DECLARE_TRACKER(tobii_tracker, tobii_dialog, tobii_metadata)
