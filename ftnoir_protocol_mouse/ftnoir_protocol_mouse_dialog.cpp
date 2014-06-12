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
#include "ftnoir_protocol_mouse.h"
#include "facetracknoir/global-settings.h"

MOUSEControls::MOUSEControls() : _proto(nullptr)
{
    ui.setupUi( this );
    ui.cbxSelectMouse_X->addItem("None");
    ui.cbxSelectMouse_X->addItem("X");
    ui.cbxSelectMouse_X->addItem("Y");
    ui.cbxSelectMouse_X->addItem("Z");
    ui.cbxSelectMouse_X->addItem("Yaw");
    ui.cbxSelectMouse_X->addItem("Pitch");
    ui.cbxSelectMouse_X->addItem("Roll");

    ui.cbxSelectMouse_Y->addItem("None");
    ui.cbxSelectMouse_Y->addItem("X");
    ui.cbxSelectMouse_Y->addItem("Y");
    ui.cbxSelectMouse_Y->addItem("Z");
    ui.cbxSelectMouse_Y->addItem("Yaw");
    ui.cbxSelectMouse_Y->addItem("Pitch");
    ui.cbxSelectMouse_Y->addItem("Roll");

    connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
    connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));

    tie_setting(s.Mouse_X, ui.cbxSelectMouse_X);
    tie_setting(s.Mouse_Y, ui.cbxSelectMouse_Y);
}

void MOUSEControls::doOK() {
    s.b->save();
    if (_proto)
        _proto->reload();
    this->close();
}

void MOUSEControls::doCancel() {
    s.b->revert();
    this->close();
}

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocolDialog* CALLING_CONVENTION GetDialog( )
{
    return new MOUSEControls;
}
