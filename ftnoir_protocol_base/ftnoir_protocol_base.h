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
* This class implements a tracker-base											*
*********************************************************************************/
/*
	Modifications (last one on top):
		
		20121115 - WVR: Added RegisterProtocol() and unRegisterProtocol() to Dialog Class
		20110415 - WVR: Added overloaded operator - and -=
*/

#ifndef FTNOIR_PROTOCOL_BASE_H
#define FTNOIR_PROTOCOL_BASE_H

#include "ftnoir_protocol_base_global.h"
#include "ftnoir_tracker_base/ftnoir_tracker_types.h"
#include <QWidget>
#include <QFrame>

struct IProtocol
{
    virtual ~IProtocol() = 0;
    virtual bool checkServerInstallationOK() = 0;
    virtual void sendHeadposeToGame( const double* headpose ) = 0;
    virtual QString getGameName() = 0;
};

inline IProtocol::~IProtocol() { }

struct IProtocolDialog
{
    virtual ~IProtocolDialog() {}
    virtual void registerProtocol(IProtocol *protocol) = 0;
    virtual void unRegisterProtocol() = 0;
};

#endif // FTNOIR_PROTOCOL_BASE_H
