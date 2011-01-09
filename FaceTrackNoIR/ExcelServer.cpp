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
* ExcelServer		ExcelServer is the Class, that communicates headpose-data	*
*					to Excel, for analysing purposes.		         			*
********************************************************************************/
/*
	Modifications (last one on top):
	20101224 - WVR: Base class is no longer inheriting QThread. sendHeadposeToGame
					is called from run() of Tracker.cpp
*/
#include <QtGui>
#include "ExcelServer.h"
#include "Tracker.h"

/** constructor **/
ExcelServer::ExcelServer( Tracker *parent ) {

	// Save the parent
	headTracker = parent;
}

/** destructor **/
ExcelServer::~ExcelServer() {
}

//
// Update Headpose in Game.
//
void ExcelServer::sendHeadposeToGame() {
}

//
// Check if the Client DLL exists and load it (to test it), if so.
// Returns 'true' if all seems OK.
//
bool ExcelServer::checkServerInstallationOK( HANDLE handle )
{   
	return true;
}

//END
