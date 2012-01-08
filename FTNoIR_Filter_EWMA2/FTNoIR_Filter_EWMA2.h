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
#ifndef INCLUDED_FTN_FILTER_EWMA2_H
#define INCLUDED_FTN_FILTER_EWMA2_H

#include "..\ftnoir_filter_base\ftnoir_filter_base.h"
#include "ui_FTNoIR_FilterControls.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EWMA Filter: Exponentially Weighted Moving Average filter with dynamic smoothing parameter
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class FTNoIR_Filter_EWMA2 : public IFilter
{
public:
	FTNoIR_Filter_EWMA2();
	~FTNoIR_Filter_EWMA2();

	void Release();
    void Initialize();
    void StartFilter();
	void FilterHeadPoseData(THeadPoseData *current_camera_position, THeadPoseData *target_camera_position, THeadPoseData *new_camera_position, bool newTarget);

	void getFilterFullName(QString *strToBeFilled);
	void getFilterShortName(QString *strToBeFilled);
	void getFilterDescription(QString *strToBeFilled);

	bool setParameterValue(const int index, const float newvalue);

private:
	THeadPoseData newHeadPose;								// Structure with new headpose

	bool	first_run;
	float	smoothing_frames_range;
	float	alpha_smoothing;
	float	prev_alpha[6];
	float	alpha[6];
	float	smoothed_alpha[6];

	//parameter list for the filter-function(s)
	enum
	{
		kMinSmoothing=0,
		kMaxSmoothing,
		kSmoothingScaleCurve,
		kNumFilterParameters								// Indicate number of parameters used
	};

	QString filterFullName;									// Filters' name and description
	QString filterShortName;
	QString filterDescription;

	QList<float>					parameterValueAsFloat;
	QList<std::pair<float,float>>	parameterRange;
	QList<float>					parameterSteps;
	QList<QString>					parameterNameAsString;
	QList<QString>					parameterValueAsString;
	QList<QString>					parameterUnitsAsString;
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
    void Initialize(QWidget *parent);
	void getFilterFullName(QString *strToBeFilled);
	void getFilterShortName(QString *strToBeFilled);
	void getFilterDescription(QString *strToBeFilled);

private:
	Ui::UICFilterControls ui;
	void loadSettings();
	void save();

	/** helper **/
	bool settingsDirty;

	QString filterFullName;									// Filters' name and description
	QString filterShortName;
	QString filterDescription;

private slots:
	void doOK();
	void doCancel();
	void settingChanged() { settingsDirty = true; };
	void settingChanged( int ) { settingsDirty = true; };
};

#endif						//INCLUDED_FTN_FILTER_EWMA2_H
//END

