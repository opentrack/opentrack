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
#include "ftnoir_filter_Accela.h"
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
	filterFullName = "Accela Filter";
	filterShortName = "Accela";
	filterDescription = "Accela Filter";

	// Load the settings from the current .INI-file
	loadSettings();

	QPoint offsetpos(100, 100);
	//if (parent) {
	//	this->move(parent->pos() + offsetpos);
	//}

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));

	// Connect sliders for reduction factor
	connect(ui.sensitivity, SIGNAL(valueChanged(double)), this, SLOT(settingChanged(double)));
	connect(ui.factor, SIGNAL(valueChanged(double)), this, SLOT(settingChanged(double)));
	connect(ui.sensitivity_translation, SIGNAL(valueChanged(double)), this, SLOT(settingChanged(double)));
	connect(ui.factor_translation, SIGNAL(valueChanged(double)), this, SLOT(settingChanged(double)));
	connect(ui.c_0, SIGNAL(valueChanged(double)), this, SLOT(settingChanged(double)));
	connect(ui.c_1, SIGNAL(valueChanged(double)), this, SLOT(settingChanged(double)));
	connect(ui.c_2, SIGNAL(valueChanged(double)), this, SLOT(settingChanged(double)));
	connect(ui.c_3, SIGNAL(valueChanged(double)), this, SLOT(settingChanged(double)));
	connect(ui.c_4, SIGNAL(valueChanged(double)), this, SLOT(settingChanged(double)));

	qDebug() << "FilterControls() says: started";
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
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "FTNoIR_Filter::loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "Filter_Accela" );
	ui.factor->setValue(iniFile.value ("factor", 8.0).toDouble());
	ui.sensitivity->setValue(iniFile.value("sensitivity", 12.0).toDouble());
	ui.factor_translation->setValue(iniFile.value ("factor_translation", 1.0).toDouble());
	ui.sensitivity_translation->setValue(iniFile.value("sensitivity_translation", 1.0).toDouble());
	ui.c_0->setValue(iniFile.value("c0", 0.005).toDouble());
	ui.c_1->setValue(iniFile.value("c1", 0.125).toDouble());
	ui.c_2->setValue(iniFile.value("c2", 0.33333).toDouble());
	ui.c_3->setValue(iniFile.value("c3", 0.75).toDouble());
	ui.c_4->setValue(iniFile.value("c4", 1.0).toDouble());
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

	iniFile.beginGroup ( "Filter_Accela" );
	iniFile.setValue ( "sensitivity", ui.sensitivity->value() );
	iniFile.setValue("factor", ui.factor->value());
	iniFile.setValue ( "sensitivity_translation", ui.sensitivity_translation->value() );
	iniFile.setValue("factor_translation", ui.factor_translation->value());
	iniFile.setValue("c0", ui.c_0->value());
	iniFile.setValue("c1", ui.c_1->value());
	iniFile.setValue("c2", ui.c_2->value());
	iniFile.setValue("c3", ui.c_3->value());
	iniFile.setValue("c4", ui.c_4->value());
	iniFile.endGroup ();

	settingsDirty = false;
}

void FilterControls::getFullName(QString *strToBeFilled)
{
	*strToBeFilled = filterFullName;
};


void FilterControls::getShortName(QString *strToBeFilled)
{
	*strToBeFilled = filterShortName;
};


void FilterControls::getDescription(QString *strToBeFilled)
{
	*strToBeFilled = filterDescription;
};

void FilterControls::getIcon(QIcon *icon)
{
	*icon = QIcon(":/images/filter-16-ac.png");
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
