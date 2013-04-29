/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2013	Wim Vriend (Developing)									*
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
/*
	Modifications (last one on top):
		20130102 - WVR: Added 'reduction factor' to accommodate Patrick's need for speed.
*/
#include "ftnoir_filter_accela/ftnoir_filter_accela.h"
#include "math.h"
#include <QDebug>
#include "facetracknoir/global-settings.h"

//*******************************************************************************************************
// FaceTrackNoIR Filter Settings-dialog.
//*******************************************************************************************************
//
// Constructor for server-settings-dialog
//
FilterControls::FilterControls() :
	QWidget(),
    functionConfig("Accela-Scaling-Rotation", 4, 8),
    translationFunctionConfig("Accela-Scaling-Translation", 4, 8)
{
	ui.setupUi( this );

	// Load the settings from the current .INI-file
	loadSettings();
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
	connect(ui.scalingConfig, SIGNAL(CurveChanged(bool)), this, SLOT(settingChanged(bool)));
	connect(ui.translationScalingConfig, SIGNAL(CurveChanged(bool)), this, SLOT(settingChanged(bool)));

	// Connect slider for reduction
    //connect(ui.slideReduction, SIGNAL(valueChanged(int)), this, SLOT(settingChanged(int)));

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
void FilterControls::Initialize(QWidget *parent, IFilter* ptr) {

	//
	// The dialog can be opened, while the Tracker is running.
	// In that case, ptr will point to the active Filter-instance.
	// This can be used to update settings, while Tracking and may also be handy to display logging-data and such...
	//
	pFilter = ptr;
    loadSettings();
	
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
	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "FTNoIR_Filter::loadSettings2 says: iniFile = " << currentFile;

    //qDebug() << "FTNoIR_Filter::loadSettings2 says: size = " << NUM_OF(defScaleRotation);

	ui.translationScalingConfig->setConfig(&translationFunctionConfig, currentFile);
	ui.scalingConfig->setConfig(&functionConfig, currentFile);

	iniFile.beginGroup ( "Accela" );
	ui.slideReduction->setValue (iniFile.value ( "Reduction", 100 ).toInt());
    ui.slideZoom->setValue(iniFile.value("zoom-slowness", 0).toInt());
	iniFile.endGroup ();

	settingsDirty = false;
}

//
// Save the current Settings to the currently 'active' INI-file.
//
void FilterControls::save() {
	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "FTNoIR_Filter::save() says: iniFile = " << currentFile;

	iniFile.beginGroup ( "Accela" );
	iniFile.setValue ( "Reduction", ui.slideReduction->value() );
    iniFile.setValue("zoom-slowness", ui.slideZoom->value());
	iniFile.endGroup ();

	functionConfig.saveSettings(iniFile);
	translationFunctionConfig.saveSettings(iniFile);

	settingsDirty = false;
}

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Filter-settings dialog object.

// Export both decorated and undecorated names.
//   GetFilterDialog     - Undecorated name, which can be easily used with GetProcAddress
//                          Win32 API function.
//   _GetFilterDialog@0  - Common name decoration for __stdcall functions in C language.
//#pragma comment(linker, "/export:GetFilterDialog=_GetFilterDialog@0")

extern "C" FTNOIR_FILTER_BASE_EXPORT IFilterDialog* CALLING_CONVENTION GetDialog()
{
    return new FilterControls;
}
