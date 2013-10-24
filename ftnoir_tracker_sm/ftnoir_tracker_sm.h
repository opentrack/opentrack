/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2012	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage:			http://facetracknoir.sourceforge.net/home/default.htm		*
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
#include <QObject>
#include <QWidget>
#include <QMessageBox>
#include <QSettings>
#include <QProcess>
#include <windows.h>
#include "math.h"
#include "facetracknoir/global-settings.h"
#include "compat/compat.h"
#include <QFrame>
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "ftnoir_tracker_sm/ftnoir_tracker_sm_types.h"
#include "ui_ftnoir_sm_controls.h"

using namespace std;

class FTNoIR_Tracker : public ITracker
{
public:
	FTNoIR_Tracker();
	~FTNoIR_Tracker();

    void StartTracker( QFrame* parent_window );
    void StopTracker( bool exit );
    bool GiveHeadPoseData(double *data);				// Returns true if confidence is good
    void WaitForExit();
	void doCommand(int foo);
	void doCommand(int foo, int bar);
	void doStartEngine(){
		doCommand(FT_SM_START);
		doCommand(FT_SM_SET_PAR_FILTER, 0);
		//doCommand(FT_SM_SHOW_CAM);
	}
	void loadSettings();

private:
	//
	// global variables
	//
    PortableLockedShm shm;
    SMMemMap *pMemData;
	QProcess *faceAPI;

	bool bEnableRoll;
	bool bEnablePitch;
	bool bEnableYaw;
	bool bEnableX;
	bool bEnableY;
	bool bEnableZ;
	bool started;
};

class TrackerControls: public QWidget, public ITrackerDialog
{
	Q_OBJECT
public:
	explicit TrackerControls();
    virtual ~TrackerControls();
	void showEvent ( QShowEvent * event );

    void Initialize(QWidget *parent);
	void registerTracker(ITracker *tracker) {
	}
	void unRegisterTracker() {
	}
protected slots:
	void doOK();
	void doCancel();
	void save();
	void settingChanged() { settingsDirty = true; }
	void settingChanged(int) { settingsDirty = true; }
private:
	Ui::UICSMClientControls ui;
	void loadSettings();
	bool settingsDirty;
};

//*******************************************************************************************************
// FaceTrackNoIR Tracker DLL. Functions used to get general info on the Tracker
//*******************************************************************************************************
class FTNoIR_TrackerDll : public Metadata
{
public:
	FTNoIR_TrackerDll();
	~FTNoIR_TrackerDll();

	void getFullName(QString *strToBeFilled);
	void getShortName(QString *strToBeFilled);
	void getDescription(QString *strToBeFilled);
	void getIcon(QIcon *icon);

private:
	QString trackerFullName;									// Trackers' name and description
	QString trackerShortName;
	QString trackerDescription;
};
