/* Copyright (c) 2023, Khoa Nguyen <khoanguyen@3forcom.com>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once
#include "api/plugin-api.hpp"
#include "ui_tobii.h"

#include <tobii/tobii.h>
#include <tobii/tobii_streams.h>

#include <QMutex>

class tobii_tracker : public ITracker
{
public:
    tobii_tracker();
    ~tobii_tracker() override;
    module_status start_tracker(QFrame*) override;
    void data(double* data) override;

private:
    tobii_api_t* api = nullptr;
    tobii_device_t* device = nullptr;

    tobii_head_pose_t latest_head_pose{
        .timestamp_us = 0LL,
        .position_validity = TOBII_VALIDITY_INVALID,
        .position_xyz = { 0.f, 0.f, 0.f },
        .rotation_validity_xyz = { TOBII_VALIDITY_INVALID, TOBII_VALIDITY_INVALID, TOBII_VALIDITY_INVALID },
        .rotation_xyz = { 0.f, 0.f, 0.f },
    };

    QMutex mtx;
};

class tobii_dialog : public ITrackerDialog
{
    Q_OBJECT

    Ui::tobii_ui ui;

public:
    tobii_dialog();

private slots:
    void doOK();
    void doCancel();
};

class tobii_metadata : public Metadata
{
    Q_OBJECT

    QString name() override { return tr("Tobii Eye Tracker"); }
    QIcon icon() override { return QIcon(":/images/tobii_logo.png"); }
};
