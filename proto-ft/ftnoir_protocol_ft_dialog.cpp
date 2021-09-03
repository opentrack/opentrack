/* Homepage         http://facetracknoir.sourceforge.net/home/default.htm        *
 *                                                                               *
 * ISC License (ISC)                                                             *
 *                                                                               *
 * Copyright (c) 2015, Wim Vriend                                                *
 *                                                                               *
 * Permission to use, copy, modify, and/or distribute this software for any      *
 * purpose with or without fee is hereby granted, provided that the above        *
 * copyright notice and this permission notice appear in all copies.             *
 */

#include "compat/library-path.hpp"
#include "ftnoir_protocol_ft.h"
#include <QDebug>
#include <QFileDialog>

FTControls::FTControls()
{
    ui.setupUi( this );

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    connect(ui.bntLocateNPClient, SIGNAL(clicked()), this, SLOT(selectDLL()));

    tie_setting(s.used_interface, ui.cbxSelectInterface);
    tie_setting(s.ephemeral_library_location, ui.ephemeral_registry_entry);
    tie_setting(s.custom_location_pathname, ui.custom_location);
    tie_setting(s.use_custom_location, ui.enable_custom_location);

    connect(ui.set_custom_location, &QAbstractButton::clicked, this, &FTControls::set_custom_location);
}

void FTControls::doOK()
{
    s.b->save();
    close();
}

void FTControls::doCancel()
{
    close();
}

void FTControls::selectDLL()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Select the desired NPClient DLL"),
                                                    OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH "/NPClient.dll",
                                                    tr("Dll file (*.dll);;All Files (*)"));

    if (!filename.isEmpty())
    {
            QSettings node("NaturalPoint", "NATURALPOINT\\NPClient Location");
            QFileInfo dllname(filename);
            node.setValue("Path", dllname.dir().path());
    }
}
void FTControls::set_custom_location()
{
    static const auto program_directory = OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH;
    auto previous_location = *s.custom_location_pathname;
    if (!s.use_custom_location || previous_location.isEmpty() || !QDir{previous_location}.exists())
        previous_location = program_directory;
    auto dir = QFileDialog::getExistingDirectory(this, tr("Select library location"), previous_location);
    if (dir.isEmpty() || !QDir{dir}.exists())
        dir = QString{};
    else
        ui.enable_custom_location->setEnabled(true);
    ui.custom_location->setText(dir);
}
