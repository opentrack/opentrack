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

//*******************************************************************************************************
// FaceTrackNoIR Client Settings-dialog.
//*******************************************************************************************************

//
// Constructor for server-settings-dialog
//
FTControls::FTControls() :
    QWidget()
{
    QString aFileName;														// File Path and Name

    ui.setupUi( this );

    // Connect Qt signals to member-functions
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    connect(ui.bntLocateNPClient, SIGNAL(clicked()), this, SLOT(selectDLL()));

    tie_setting(s.useDummyExe, ui.chkStartDummy);
    tie_setting(s.useTIRViews, ui.chkTIRViews);

    ui.cbxSelectInterface->addItem("Enable both");
    ui.cbxSelectInterface->addItem("Use FreeTrack, hide TrackIR");
    ui.cbxSelectInterface->addItem("Use TrackIR, hide FreeTrack");

    tie_setting(s.intUsedInterface, ui.cbxSelectInterface);

    aFileName = QCoreApplication::applicationDirPath() + "/TIRViews.dll";
    if ( !QFile::exists( aFileName ) ) {
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
    s.b->revert();
    this->close();
}

void FTControls::selectDLL() {
    QString fileName = QFileDialog::getOpenFileName( this, tr("Select the desired NPClient DLL"), QCoreApplication::applicationDirPath() + "/NPClient.dll", tr("Dll file (*.dll);;All Files (*)"));

    //
    // Write the location of the file in the required Registry-key.
    //
    if (! fileName.isEmpty() ) {
        if (fileName.endsWith("NPClient.dll", Qt::CaseInsensitive) ) {
            QSettings settingsTIR("NaturalPoint", "NATURALPOINT\\NPClient Location");			// Registry settings (in HK_USER)
            QString aLocation = fileName.left(fileName.length() - 12);							// Location of Client DLL

            settingsTIR.setValue( "Path" , aLocation );
        }
    }
}

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocolDialog* CALLING_CONVENTION GetDialog( )
{
    return new FTControls;
}
