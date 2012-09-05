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

#include "..\ftnoir_filter_base\ftnoir_filter_base.h"
#include "ui_FTNoIR_FilterControls.h"
#include <FunctionConfig.h>

//*******************************************************************************************************
// FaceTrackNoIR Filter class.
//*******************************************************************************************************
class FTNoIR_Filter : public IFilter
{
public:
	FTNoIR_Filter();
	~FTNoIR_Filter();

	void Release();
    void Initialize();
    void StartFilter();
	void FilterHeadPoseData(THeadPoseData *current_camera_position, THeadPoseData *target_camera_position, THeadPoseData *new_camera_position, bool newTarget);

private:
	void loadSettings();									// Load the settings from the INI-file
	THeadPoseData newHeadPose;								// Structure with new headpose

	bool	first_run;
	double kFactor, kFactorTranslation;
	double kSensitivity, kSensitivityTranslation;

	FunctionConfig functionConfig;
	FunctionConfig translationFunctionConfig;
};

//*******************************************************************************************************
// FaceTrackNoIR Filter Settings-dialog.
//*******************************************************************************************************

// Widget that has controls for FTNoIR protocol filter-settings.
class FilterControls: public QWidget, Ui::UICFilterControls, public IFilterDialog
{
    Q_OBJECT
public:

	explicit FilterControls();
    virtual ~FilterControls();
	void showEvent ( QShowEvent * event );

	void Release();											// Member functions which are accessible from outside the DLL
    void Initialize(QWidget *parent, IFilterPtr ptr);

private:
	Ui::UICFilterControls ui;
	void loadSettings();
	void save();

	/** helper **/
	bool settingsDirty;

	IFilterPtr pFilter;										// If the filter was active when the dialog was opened, this will hold a pointer to the Filter instance
	FunctionConfig functionConfig;
	FunctionConfig translationFunctionConfig;

private slots:
	void doOK();
	void doCancel();
	void settingChanged(bool) { settingsDirty = true; };
};

//*******************************************************************************************************
// FaceTrackNoIR Filter DLL. Functions used to get general info on the Filter
//*******************************************************************************************************
class FTNoIR_FilterDll : public IFilterDll
{
public:
	FTNoIR_FilterDll();
	~FTNoIR_FilterDll();

	void Release();
    void Initialize();

	void getFullName(QString *strToBeFilled);
	void getShortName(QString *strToBeFilled);
	void getDescription(QString *strToBeFilled);
	void getIcon(QIcon *icon);

private:
	QString filterFullName;									// Filters' name and description
	QString filterShortName;
	QString filterDescription;
};


#endif						//INCLUDED_FTN_FILTER_H
//END

