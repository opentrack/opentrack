/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2010	Wim Vriend (Developing)									*
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
* FGServer			FGServer is the Class, that communicates headpose-data		*
*					to FlightGear, using UDP.				         			*
*					It is based on the (Linux) example made by Melchior FRANZ.	*
********************************************************************************/
#include "ftnoir_protocol_fg.h"
#include <QObject>
#include <QFile>
#include "facetracknoir/global-settings.h"

//*******************************************************************************************************
// FaceTrackNoIR Client Settings-dialog.
//*******************************************************************************************************

//
// Constructor for server-settings-dialog
//
FGControls::FGControls() : theProtocol(nullptr)
{
	ui.setupUi( this );

    tie_setting(s.ip1, ui.spinIPFirstNibble);
    tie_setting(s.ip2, ui.spinIPSecondNibble);
    tie_setting(s.ip3, ui.spinIPThirdNibble);
    tie_setting(s.ip4, ui.spinIPFourthNibble);
    tie_setting(s.port, ui.spinPortNumber);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
}

void FGControls::doOK() {
    s.b->save();
	this->close();
    if (theProtocol)
        theProtocol->reloadSettings();
}

void FGControls::doCancel() {
    s.b->revert();
    this->close();
}

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocolDialog* CALLING_CONVENTION GetDialog( )
{
    return new FGControls;
}
