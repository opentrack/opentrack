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
#include <QDir>
#include <QStringList>
#include <memory>

#if defined(_WIN32) && defined(_MSC_VER)
#   include <windows.h>
#	ifdef OPENTRACK_BREAKPAD
#		include <exception_handler.h>
using namespace google_breakpad;
bool dumpCallback(const wchar_t* dump_path,
                                 const wchar_t* minidump_id,
                                 void* context,
                                 EXCEPTION_POINTERS* exinfo,
                                 MDRawAssertionInfo* assertion,
                                 bool succeeded)
{
    MessageBoxA(GetDesktopWindow(),
        "Generating crash dump!\r\n"
        "Please send the .dmp file to <sthalik@misaki.pl> to help us improve the code.",
        "opentrack crashed :(",
        MB_OK | MB_ICONERROR);
	return succeeded;
}

#	endif
#endif

int main(int argc, char** argv)
{
#if defined(OPENTRACK_BREAKPAD) && defined(_MSC_VER)
	auto handler = new ExceptionHandler(L".", nullptr, dumpCallback, nullptr, -1);
#endif
    QApplication::setAttribute(Qt::AA_X11InitThreads, true);
    QApplication app(argc, argv);
    auto w = std::make_shared<FaceTrackNoIR>();

    w->show();
    app.exec();

	return 0;
}

