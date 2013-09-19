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
#pragma once
#ifndef INCLUDED_FTN_FILTER_H
#define INCLUDED_FTN_FILTER_H

#include "ftnoir_filter_base/ftnoir_filter_base.h"
#include "ui_ftnoir_accela_filtercontrols.h"
#include "facetracknoir/global-settings.h"
#include <QMutex>

#define ACCELA_SMOOTHING_ROTATION 60.0
#define ACCELA_SMOOTHING_TRANSLATION 40.0
#define ACCELA_ZOOM_SLOWNESS 35

//*******************************************************************************************************
// FaceTrackNoIR Filter class.
//*******************************************************************************************************
class FTNOIR_FILTER_BASE_EXPORT FTNoIR_Filter : public IFilter
{
public:
	FTNoIR_Filter();
	~FTNoIR_Filter();
    void FilterHeadPoseData(const double* target_camera_position, double *new_camera_position, const double* last_post_filter_values);
    void Initialize() {
        first_run = true;
    }
    void receiveSettings(double rot, double trans, double zoom_fac);
private:
    QMutex mutex;
	void loadSettings();
	bool first_run;
    double rotation_alpha, translation_alpha, zoom_factor;
    double current_camera_position[6];
};

//*******************************************************************************************************
// FaceTrackNoIR Filter Settings-dialog.
//*******************************************************************************************************

// Widget that has controls for FTNoIR protocol filter-settings.
class FTNOIR_FILTER_BASE_EXPORT FilterControls: public QWidget, public IFilterDialog
{
    Q_OBJECT
public:
	explicit FilterControls();
    virtual ~FilterControls();
	void showEvent ( QShowEvent * event );
    void Initialize(QWidget *parent);
    void registerFilter(IFilter* filter);
    void unregisterFilter();
private:
    Ui::AccelaUICFilterControls ui;
	void loadSettings();
	void save();
	bool settingsDirty;
    FTNoIR_Filter* accela_filter;
private slots:
	void doOK();
	void doCancel();
	void settingChanged(bool) { settingsDirty = true; }
	void settingChanged(int) { settingsDirty = true; }
    void settingChanged(double) { settingsDirty = true; }
};

//*******************************************************************************************************
// FaceTrackNoIR Filter DLL. Functions used to get general info on the Filter
//*******************************************************************************************************
class FTNoIR_FilterDll : public Metadata
{
public:
	FTNoIR_FilterDll();
	~FTNoIR_FilterDll();

    void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("Accela Filter Mk3"); }
    void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("Accela Mk3"); }
    void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("Accela filter Mk3"); }

    void getIcon(QIcon *icon){ *icon = QIcon(":/images/filter-16.png");	}
};


#endif						//INCLUDED_FTN_FILTER_H
//END

