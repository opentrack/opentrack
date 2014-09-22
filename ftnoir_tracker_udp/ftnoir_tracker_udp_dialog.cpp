/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2012	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage:			http://facetracknoir.sourceforge.net/home/default.htm		*
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
#include "ftnoir_tracker_udp.h"
#include "facetracknoir/plugin-support.h"

TrackerControls::TrackerControls() :
QWidget()
{
	ui.setupUi( this );

	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
	connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.port, ui.spinPortNumber);
}

void TrackerControls::doOK() {
    s.b->save();
	this->close();
}

void TrackerControls::doCancel() {
    s.b->revert();
    this->close();
}

extern "C" OPENTRACK_EXPORT ITrackerDialog* CALLING_CONVENTION GetDialog( )
{
    return new TrackerControls;
}
