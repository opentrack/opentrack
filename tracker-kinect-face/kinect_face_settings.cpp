/* Copyright (c) 2014, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "kinect_face_settings.h"
#include "kinect_face_tracker.h"
#include "api/plugin-api.hpp"
#include "compat/math-imports.hpp"

#include <QPushButton>

#include <cmath>
#include <QDebug>


KinectFaceSettings::KinectFaceSettings()
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
}

void KinectFaceSettings::doOK()
{
    //s.b->save();
    close();
}

void KinectFaceSettings::doCancel()
{
    close();
}

OPENTRACK_DECLARE_TRACKER(KinectFaceTracker, KinectFaceSettings, KinectFaceMetadata)
