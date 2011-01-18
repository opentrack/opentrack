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
#include <Windows.h>

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
SYSTEMTIME now;

	//
	// Get the System-time and substract the time from the previous call.
	// dT will be used for the EWMA-filter.
	//
	GetSystemTime ( &now );
	long newHeadPoseTime = (((now.wHour * 3600) + (now.wMinute * 60) + now.wSecond) * 1000) + now.wMilliseconds;
	
	//	Use this for some debug-output to file...
	QFile data(QCoreApplication::applicationDirPath() + "\\output.txt");
	if (data.open(QFile::WriteOnly | QFile::Append)) {
		QTextStream out(&data);
		out << newHeadPoseTime << "\t" << newSample << "\t" << confidence << "\t" << dT << "\t" << smoothvalue << "\t" << headRotX << "\t" << virtRotX << "\t" << prev_value << '\n';
	}
}

//
// Check if the Client DLL exists and load it (to test it), if so.
// Returns 'true' if all seems OK.
//
bool ExcelServer::checkServerInstallationOK( HANDLE handle )
{   
	//	Use this for some debug-output to file...
	QFile data(QCoreApplication::applicationDirPath() + "\\output.txt");
	if (data.open(QFile::WriteOnly)) {


		//
		// Get the settings for the header
		//
		QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

		QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
		QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

		iniFile.beginGroup ( "Tracking" );
		int smoothing = iniFile.value ( "Smooth", 10 ).toInt();
		bool useEWMA = iniFile.value ( "useEWMA", 1 ).toBool();
		iniFile.endGroup ();

		QTextStream out(&data);
		out << "Smoothing = \t" << smoothing << "\n" << "EWMA used = \t" << useEWMA << "\n" << "\n"; 
		out << "Time"   << "\t" << "New Sample" << "\t" << "Confidence" << "\t" << "dT"         << "\t" << "Smoothed"    << "\t" << "RotX (Pitch)" << "\t" << "RotX (Pitch)" << "\t" << "Previous" << '\n';
		out << "(long)" << "\t" << "(bool)"     << "\t" << "(float)"    << "\t" << "(float)"    << "\t"    << "(float)"  << "\t" << "Raw"          << "\t" << "Filtered"     << "\t" << "-" << '\n';
	}

	return true;
}

//END
