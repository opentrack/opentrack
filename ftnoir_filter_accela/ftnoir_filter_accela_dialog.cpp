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
#include "ftnoir_filter_accela/ftnoir_filter_accela.h"
#include <cmath>
#include <QDebug>
#include <algorithm>
#include <QDoubleSpinBox>
#include "facetracknoir/global-settings.h"

//*******************************************************************************************************
// FaceTrackNoIR Filter Settings-dialog.
//*******************************************************************************************************
//
// Constructor for server-settings-dialog
//
FilterControls::FilterControls() :
    QWidget(), accela_filter(NULL)
{
    ui.setupUi( this );

	// Load the settings from the current .INI-file
	loadSettings();
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    connect(ui.rotation_alpha, SIGNAL(valueChanged(double)), this, SLOT(settingChanged(double)));
    connect(ui.translation_alpha, SIGNAL(valueChanged(double)), this, SLOT(settingChanged(double)));

    connect(ui.spinZoom, SIGNAL(valueChanged(int)), this, SLOT(settingChanged(int)));

    QDoubleSpinBox* boxen[] = {
        ui.doubleSpinBox,
        ui.doubleSpinBox_2,
        ui.doubleSpinBox_3,
        ui.doubleSpinBox_4,
        ui.doubleSpinBox_5,
        ui.doubleSpinBox_6,
    };

    for (int i = 0; i < 6; i++)
    {
        connect(boxen[i], SIGNAL(valueChanged(double)), this, SLOT(settingChanged(double)));
    }

    connect(ui.expt, SIGNAL(valueChanged(double)), this, SLOT(settingChanged(double)));

	qDebug() << "FilterControls() says: started";
}

//
// Destructor for server-dialog
//
FilterControls::~FilterControls() {
	qDebug() << "~FilterControls() says: started";
}

//
// Initialize tracker-client-dialog
//
void FilterControls::Initialize(QWidget *parent) {
    loadSettings();
    
	QPoint offsetpos(100, 100);
	if (parent) {
		this->move(parent->pos() + offsetpos);
	}
	show();
}

void FilterControls::registerFilter(IFilter* filter)
{
    accela_filter = (FTNoIR_Filter*) filter;
}

void FilterControls::unregisterFilter()
{
    accela_filter = NULL;
}

//
// OK clicked on server-dialog
//
void FilterControls::doOK() {
	save();
	this->close();
}

// override show event
void FilterControls::showEvent ( QShowEvent *  ) {
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

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "FTNoIR_Filter::loadSettings2 says: iniFile = " << currentFile;

    //qDebug() << "FTNoIR_Filter::loadSettings2 says: size = " << NUM_OF(defScaleRotation);

	iniFile.beginGroup ( "Accela" );
    ui.spinZoom->setValue(iniFile.value("zoom-slowness", ACCELA_ZOOM_SLOWNESS).toInt());
    ui.rotation_alpha->setValue(iniFile.value("rotation-alpha", ACCELA_SMOOTHING_ROTATION).toDouble());
    ui.translation_alpha->setValue(iniFile.value("translation-alpha", ACCELA_SMOOTHING_TRANSLATION).toDouble());
    ui.order_2nd->setValue(iniFile.value("second-order-alpha", ACCELA_SECOND_ORDER_ALPHA).toDouble());
    ui.order_3rd->setValue(iniFile.value("third-order-alpha", ACCELA_THIRD_ORDER_ALPHA).toDouble());
    ui.deadzone->setValue(iniFile.value("deadzone", 0).toDouble());

    // bigger means less filtering
    static const double init_scaling[] = {
        1.5, // X
        1.5, // Y
        1,   // Z
        0.8, // Yaw
        0.9, // Pitch
        1.25 // Roll
    };

    QDoubleSpinBox* boxen[] = {
        ui.doubleSpinBox,
        ui.doubleSpinBox_2,
        ui.doubleSpinBox_3,
        ui.doubleSpinBox_4,
        ui.doubleSpinBox_5,
        ui.doubleSpinBox_6,
    };

    for (int i = 0; i < 6; i++)
    {
        boxen[i]->setValue(iniFile.value(QString("axis-%1").arg(QString::number(i)), init_scaling[i]).toDouble());
    }

    ui.expt->setValue(iniFile.value("exponent", 2.0).toDouble());

    iniFile.endGroup();

	settingsDirty = false;
}

//
// Save the current Settings to the currently 'active' INI-file.
//
void FilterControls::save() {
	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
    {
        QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

        qDebug() << "FTNoIR_Filter::save() says: iniFile = " << currentFile;

        iniFile.beginGroup ( "Accela" );
        iniFile.setValue("rotation-alpha", ui.rotation_alpha->value());
        iniFile.setValue("translation-alpha", ui.translation_alpha->value());
        iniFile.setValue("zoom-slowness", ui.spinZoom->value());
        iniFile.setValue("deadzone", ui.deadzone->value());
        iniFile.setValue("exponent", ui.expt->value());
        iniFile.setValue("second-order-alpha", ui.order_2nd->value());
        iniFile.setValue("third-order-alpha", ui.order_3rd->value());

        QDoubleSpinBox* boxen[] = {
            ui.doubleSpinBox,
            ui.doubleSpinBox_2,
            ui.doubleSpinBox_3,
            ui.doubleSpinBox_4,
            ui.doubleSpinBox_5,
            ui.doubleSpinBox_6,
        };

        for (int i = 0; i < 6; i++)
        {
            iniFile.setValue(QString("axis-%1").arg(QString::number(i)), boxen[i]->value());
        }
        iniFile.endGroup();
    }

	settingsDirty = false;
    
    if (accela_filter)
        accela_filter->receiveSettings();
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
