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
#pragma once
#ifndef INCLUDED_EXCELSERVER_H
#define INCLUDED_EXCELSERVER_H
 
#include "FTNoIR_cxx_protocolserver.h"
#include <QString>
#include <QMessageBox>
#include <QSettings>
#include <QFile>
#include <QApplication>
#include <QDebug>
#include <QMutex>
#include <QLibrary>

using namespace std;
using namespace v4friend::ftnoir;

class Tracker;				// pre-define parent-class to avoid circular includes

class ExcelServer : public ProtocolServerBase {
	Q_OBJECT

public: 

	// public member methods
	ExcelServer( Tracker *parent );
	~ExcelServer();

	// protected member methods
protected:
	bool checkServerInstallationOK( HANDLE handle );
	void sendHeadposeToGame();

private:
	Tracker *headTracker;									// For upstream messages...
};

#endif//INCLUDED_EXCELSERVER_H
//END
