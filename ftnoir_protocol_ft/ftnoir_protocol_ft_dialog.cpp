/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2012	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage																		*
*																				*
* This program is free software; you can redistribute it and/or modify it		*
* under the terms of the GNU General Public License as published by the			*
* Free Software Foundation; either version 3 of the License, or (at your		*
* option) any later version.													*
*																				*
* This program is distributed in the hope that it will be useful, but			*
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY	*
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for	*
* more details.																	*
*																				*
* You should have received a copy of the GNU General Public License along		*
* with this program; if not, see <http://www.gnu.org/licenses/>.				*
*																				*
********************************************************************************/
#include "ftnoir_protocol_ft.h"
#include <QDebug>
#include <QFileDialog>

FTControls::FTControls()
{
    ui.setupUi( this );

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    connect(ui.bntLocateNPClient, SIGNAL(clicked()), this, SLOT(selectDLL()));

    tie_setting(s.useTIRViews, ui.chkTIRViews);

    ui.cbxSelectInterface->addItem("Enable both");
    ui.cbxSelectInterface->addItem("Use FreeTrack, hide TrackIR");
    ui.cbxSelectInterface->addItem("Use TrackIR, hide FreeTrack");

    tie_setting(s.intUsedInterface, ui.cbxSelectInterface);

    QFile memhacks_pathname(QCoreApplication::applicationDirPath() + "/TIRViews.dll");
    if (!memhacks_pathname.exists()) {
        ui.chkTIRViews->setChecked( false );
        ui.chkTIRViews->setEnabled ( false );
    }
    else {
        ui.chkTIRViews->setEnabled ( true );
    }
}

void FTControls::doOK() {
    s.b->save();
    this->close();
}

void FTControls::doCancel() {
    s.b->reload();
    this->close();
}

void FTControls::selectDLL() {
    QString filename = QFileDialog::getOpenFileName( this, tr("Select the desired NPClient DLL"), QCoreApplication::applicationDirPath() + "/NPClient.dll", tr("Dll file (*.dll);;All Files (*)"));

    if (! filename.isEmpty() ) {
            QSettings node("NaturalPoint", "NATURALPOINT\\NPClient Location");
            QFileInfo dllname(filename);
            node.setValue( "Path" , dllname.dir().path() );
    }
}

extern "C" OPENTRACK_EXPORT IProtocolDialog* GetDialog()
{
    return new FTControls;
}
