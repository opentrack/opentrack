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
#include "ftnoir_filter_EWMA2.h"
#include "math.h"
#include <QDebug>

//*******************************************************************************************************
// FaceTrackNoIR Filter Settings-dialog.
//*******************************************************************************************************
//
// Constructor for server-settings-dialog
//
FilterControls::FilterControls() :
QWidget()
{
	ui.setupUi( this );

	//populate the description strings
	filterFullName = "EWMA Filter Mk2";
	filterShortName = "EWMA";
	filterDescription = "Exponentially Weighted Moving Average filter with dynamic smoothing parameter";

	QPoint offsetpos(100, 100);
	//if (parent) {
	//	this->move(parent->pos() + offsetpos);
	//}

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));

	// Connect sliders for reduction factor
	connect(ui.minSmooth, SIGNAL(valueChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.maxSmooth, SIGNAL(valueChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.powCurve, SIGNAL(valueChanged(int)), this, SLOT(settingChanged(int)));

	// Load the settings from the current .INI-file
	loadSettings();
}

//
// Destructor for server-dialog
//
FilterControls::~FilterControls() {
	qDebug() << "~FilterControls() says: started";
}

void FilterControls::Release()
{
    delete this;
}

//
// Initialize tracker-client-dialog
//
void FilterControls::Initialize(QWidget *parent) {

	QPoint offsetpos(100, 100);
	if (parent) {
		this->move(parent->pos() + offsetpos);
	}
	show();
}

//
// OK clicked on server-dialog
//
void FilterControls::doOK() {
	save();
	this->close();
}

// override show event
void FilterControls::showEvent ( QShowEvent * event ) {
	loadSettings();
}

//
// Cancel clicked on server-dialog
//
void FilterControls::doCancel() {
	//
	// Ask if changed Settings should be saved
	//
	if (settingsDirty) {
		int ret = QMessageBox::question ( this, "Settings have changed", "Do you want to save the settings?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Discard );

		qDebug() << "doCancel says: answer =" << ret;

		switch (ret) {
			case QMessageBox::Save:
				save();
				this->close();
				break;
			case QMessageBox::Discard:
				this->close();
				break;
			case QMessageBox::Cancel:
				// Cancel was clicked
				break;
			default:
				// should never be reached
			break;
		}
	}
	else {
		this->close();
	}
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FilterControls::loadSettings() {
//	qDebug() << "loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

//	qDebug() << "loadSettings says: iniFile = " << currentFile;

	//
	// The EWMA2-filter-settings are in the Tracking group: this is because they used to be on the Main Form of FaceTrackNoIR
	//
	iniFile.beginGroup ( "Tracking" );
	ui.minSmooth->setValue (iniFile.value ( "minSmooth", 15 ).toInt());
	ui.maxSmooth->setValue (iniFile.value ( "maxSmooth", 50 ).toInt());
	ui.powCurve->setValue (iniFile.value ( "powCurve", 10 ).toInt());
	iniFile.endGroup ();

	settingsDirty = false;
}

//
// Save the current Settings to the currently 'active' INI-file.
//
void FilterControls::save() {
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "Tracking" );
	iniFile.setValue ( "minSmooth", ui.minSmooth->value() );
	iniFile.setValue ( "powCurve", ui.powCurve->value() );
	iniFile.setValue ( "maxSmooth", ui.maxSmooth->value() );
	iniFile.endGroup ();

	settingsDirty = false;
}

void FilterControls::getFilterFullName(QString *strToBeFilled)
{
	*strToBeFilled = filterFullName;
};


void FilterControls::getFilterShortName(QString *strToBeFilled)
{
	*strToBeFilled = filterShortName;
};


void FilterControls::getFilterDescription(QString *strToBeFilled)
{
	*strToBeFilled = filterDescription;
};

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Filter-settings dialog object.

// Export both decorated and undecorated names.
//   GetFilterDialog     - Undecorated name, which can be easily used with GetProcAddress
//                          Win32 API function.
//   _GetFilterDialog@0  - Common name decoration for __stdcall functions in C language.
#pragma comment(linker, "/export:GetFilterDialog=_GetFilterDialog@0")

FTNOIR_FILTER_BASE_EXPORT FILTERDIALOGHANDLE __stdcall GetFilterDialog( )
{
	return new FilterControls;
}
