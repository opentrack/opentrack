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
#include "ftnoir_protocol_fsuipc.h"
#include "facetracknoir/global-settings.h"

FSUIPCControls::FSUIPCControls() :
    QWidget()
{
    ui.setupUi( this );
    connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
    connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
    connect(ui.btnFindDLL, SIGNAL(clicked()), this, SLOT(getLocationOfDLL()));

    tie_setting(s.LocationOfDLL, ui.txtLocationOfDLL);
}

void FSUIPCControls::doOK() {
    s.b->save();
    this->close();
}

void FSUIPCControls::doCancel() {
    s.b->revert();
    close();
}

void FSUIPCControls::getLocationOfDLL()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Locate file"),
                                                    ui.txtLocationOfDLL->text(),
                                                    tr("FSUIPC DLL file (FSUIPC*.dll);;All Files (*)"));
    if (!fileName.isEmpty()) {
        s.LocationOfDLL = fileName;
    }
}

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocolDialog* CALLING_CONVENTION GetDialog(void)
{
    return new FSUIPCControls;
}
