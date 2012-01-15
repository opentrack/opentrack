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
#include "ftnoir_filter_DZ1.h"
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
	filterFullName = "Deadzone Filter Mk1";
	filterShortName = "DZ1";
	filterDescription = "Deadzone Filter";

	QPoint offsetpos(100, 100);
	//if (parent) {
	//	this->move(parent->pos() + offsetpos);
	//}

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));

	// Connect sliders for reduction factor
	connect(ui.slideHz, SIGNAL(valueChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.spinDeadZone, SIGNAL(valueChanged(double)), this, SLOT(settingChanged(double)));
	connect(ui.slideMoveLast, SIGNAL(valueChanged(int)), this, SLOT(settingChanged(int)));

	qDebug() << "FilterControls() says: started";

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
void FilterControls::Initialize(QWidget *parent, IFilterPtr ptr) {

	//
	// The dialog can be opened, while the Tracker is running.
	// In that case, ptr will point to the active Filter-instance.
	// This can be used to update settings, while Tracking and may also be handy to display logging-data and such...
	//
	pFilter = ptr;
	
	//
	//
	//
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
	if (pFilter) {
		pFilter->Initialize();
	}
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
	qDebug() << "FilterControls::loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "FilterControls::loadSettings says: iniFile = " << currentFile;

	//
	// The DZ1-filter-settings
	//
	iniFile.beginGroup ( "Filter_DZ1" );
	ui.slideHz->setValue (iniFile.value ( "cameraHz", 30 ).toInt());
	ui.spinDeadZone->setValue (iniFile.value ( "DeadZone", 0.1f ).toDouble());
	ui.slideMoveLast->setValue (iniFile.value ( "MoveLast", 24 ).toInt());
	ui.spinMaxDiff->setValue (iniFile.value ( "MaxDiff", 1.75f ).toDouble());
	ui.slideMoveSaved->setValue (iniFile.value ( "MoveSaved", 35 ).toFloat());
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

	iniFile.beginGroup ( "Filter_DZ1" );
	iniFile.setValue ( "cameraHz", ui.slideHz->value() );
	iniFile.setValue ( "DeadZone", ui.spinDeadZone->value() );
	iniFile.setValue ( "MoveLast", ui.slideMoveLast->value() );
	iniFile.setValue ( "MaxDiff", ui.spinMaxDiff->value() );
	iniFile.setValue ( "MoveSaved", ui.slideMoveSaved->value() );
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

void FilterControls::getIcon(QIcon *icon)
{
	*icon = QIcon(":/images/filter-16.png");
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
