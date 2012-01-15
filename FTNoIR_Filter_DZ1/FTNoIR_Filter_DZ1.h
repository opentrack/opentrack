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

static int compare_double(const void *one, const void *two) {
	double foo = (*((double*) one) - *((double*) two));
	if (foo > 0)
		return 1;
	if (foo < 0)
		return -1;
	return 0;
}

//#define HZ 30

//#define DEADZONE 0.1
//#define MOVE_LAST 0.24
//#define MOVE_SAVED (0.35 / HZ)
#define SLOW_SPEED 0.1

//#define MAXDIFF 1.75
#define INITIAL_SMOOTH_SPEED 0.00834
#define SMOOTH_FACTOR 2.5
#define REMEMBER_SMOOTHNESS 5					// Changed from HZ/5

#define MULT_Y_POS 1.7
#define MULT_Y_NEG 0.8
#define MULT_X 1.5

#define COCKPIT_PITCH 8.7

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DZ1 Filter: Stanislaw wrote a new filter, which makes the view more stable, when not moving your face.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class FTNoIR_Filter : public IFilter
{
public:
	FTNoIR_Filter();
	~FTNoIR_Filter();

	void Release();
    void Initialize();
    void StartFilter();
	void FilterHeadPoseData(THeadPoseData *current_camera_position, THeadPoseData *target_camera_position, THeadPoseData *new_camera_position, bool newTarget);

	void getFilterFullName(QString *strToBeFilled);
	void getFilterShortName(QString *strToBeFilled);
	void getFilterDescription(QString *strToBeFilled);

private:
	void loadSettings();									// Load the settings from the INI-file
	THeadPoseData newHeadPose;								// Structure with new headpose

	bool	first_run;
	double  last_positions[6];
	double  saved_positions[6];
	double  prev_positions[6];
	bool    smoothing[6];
	double  smooth_start[6];
	double  smooth_speed[6];
	int		smooth_remember[6];

	float	kCameraHz;										// Hz
	float	kDeadZone;										// degrees
	float	kMoveLast;										// %
	float	kMaxDiff;										// degrees
	float	kMoveSaved;										// %

	QString filterFullName;									// Filters' name and description
	QString filterShortName;
	QString filterDescription;
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
	void getFilterFullName(QString *strToBeFilled);
	void getFilterShortName(QString *strToBeFilled);
	void getFilterDescription(QString *strToBeFilled);
	void getIcon(QIcon *icon);

private:
	Ui::UICFilterControls ui;
	void loadSettings();
	void save();

	/** helper **/
	bool settingsDirty;

	QString filterFullName;									// Filters' name and description
	QString filterShortName;
	QString filterDescription;
	IFilterPtr pFilter;										// If the filter was active when the dialog was opened, this will hold a pointer to the Filter instance

private slots:
	void doOK();
	void doCancel();
	void settingChanged() { settingsDirty = true; };
	void settingChanged( int ) { settingsDirty = true; };
	void settingChanged( double ) { settingsDirty = true; };
};

#endif						//INCLUDED_FTN_FILTER_H
//END

