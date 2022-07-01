/* Copyright (c) 2019, Stephane Lenclud <github@lenclud.com>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once
#include "ui_kinect_face_settings.h"
#include "api/plugin-api.hpp"

class KinectFaceSettings : public ITrackerDialog
{
    Q_OBJECT

    Ui::KinectFaceUi ui;

public:
    KinectFaceSettings();
    void register_tracker(ITracker *) override {}
    void unregister_tracker() override {}
};

class KinectFaceMetadata : public Metadata
{
    Q_OBJECT

    QString name() override { return tr("Kinect Face 0.1"); }
    QIcon icon() override { return QIcon(":/images/kinect.png"); }
};

