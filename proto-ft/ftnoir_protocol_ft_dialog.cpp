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
#include "ftnoir_protocol_ft.h"
#include "opentrack-library-path.h"
#include <QDebug>
#include <QFileDialog>

FTControls::FTControls()
{
    ui.setupUi( this );

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    connect(ui.bntLocateNPClient, SIGNAL(clicked()), this, SLOT(selectDLL()));

    tie_setting(s.useTIRViews, ui.chkTIRViews);
    tie_setting(s.close_protocols_on_exit, ui.close_on_exit);

    ui.cbxSelectInterface->addItem("Enable both");
    ui.cbxSelectInterface->addItem("Use FreeTrack, hide TrackIR");
    ui.cbxSelectInterface->addItem("Use TrackIR, hide FreeTrack");

    tie_setting(s.intUsedInterface, ui.cbxSelectInterface);

    static const QFile memhacks_pathname(OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH "TIRViews.dll");
    if (!memhacks_pathname.exists()) {
        ui.chkTIRViews->setEnabled(false);
    }
    else {
        ui.chkTIRViews->setEnabled(true);
    }
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

