/* Copyright (c) 2019, Stephane Lenclud <github@lenclud.com>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "kinect_face_settings.h"
#include "kinect_face_tracker.h"
#include "api/plugin-api.hpp"
#include "compat/math-imports.hpp"
#include "compat/library-path.hpp"

#include <cmath>

#include <QDesktopServices>
#include <QUrl>
#include <QPushButton>
#include <QDebug>

KinectFaceSettings::KinectFaceSettings()
{
    ui.setupUi(this);

    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &KinectFaceSettings::close);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &KinectFaceSettings::close);

    static const QUrl path {"file:///" + application_base_path() + OPENTRACK_DOC_PATH "/3rdparty-notices/Kinect-V2-SDK-Eula.rtf" };

    connect(ui.buttonBox, &QDialogButtonBox::helpRequested, [] {
        QDesktopServices::openUrl(path);
    });

    ui.buttonBox->addButton(tr("Kinect license"), QDialogButtonBox::HelpRole);
}

OPENTRACK_DECLARE_TRACKER(KinectFaceTracker, KinectFaceSettings, KinectFaceMetadata)
