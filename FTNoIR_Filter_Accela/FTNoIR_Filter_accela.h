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

const QPointF defScaleRotation[] =
{
    QPointF(0, 0),
    QPointF(0.308900523560209, 0.0666666666666667),
    QPointF(0.565445026178011, 0.226666666666667),
    QPointF(0.769633507853403, 0.506666666666667),
    QPointF(0.994764397905759, 1),
    QPointF(1.23560209424084, 1.61333333333333),
    QPointF(1.47643979057592, 2.37333333333333),
    QPointF(1.66492146596859, 3.12),
    QPointF(1.80628272251309, 3.92),
    QPointF(1.91623036649215, 4.70666666666667),
    QPointF(2.00523560209424, 5.44),
    QPointF(2.07329842931937, 6)
};

const QPointF defScaleTranslation[] =
{
    QPointF(0, 0),
    QPointF(0.282722513089005, 0.08),
    QPointF(0.492146596858639, 0.306666666666667),
    QPointF(0.764397905759162, 0.84),
	QPointF(1.00523560209424, 1.62666666666667),
	QPointF(1.17277486910995, 2.78666666666667),
	QPointF(1.25130890052356, 3.6),
	QPointF(1.31937172774869, 4.29333333333333),
	QPointF(1.38219895287958, 4.90666666666667),
    QPointF(1.43455497382199, 5.65333333333333)
};

//
// Macro to determine array-size
//
#define NUM_OF(x) (sizeof (x) / sizeof *(x))

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

