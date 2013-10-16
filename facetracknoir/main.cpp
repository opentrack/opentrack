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
*********************************************************************************/

#include "facetracknoir.h"
#include "tracker.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QList>

#if defined(_WIN32)
#   include <windows.h>
//#pragma comment(linker, "/SUBSYSTEM:console /ENTRY:mainCRTStartup")
#endif

int main(int argc, char** argv)
{
    QApplication::setAttribute(Qt::AA_X11InitThreads, true);

    QApplication app(argc, argv);

    FaceTrackNoIR w;
	QDesktopWidget desktop;

    w.move(desktop.screenGeometry().width()/2-w.width()/2, 100);
	w.show();
    app.exec();

	return 0;
}

