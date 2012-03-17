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
#include "..\ftnoir_tracker_base\ftnoir_tracker_base.h"
#include "..\ftnoir_tracker_base\ftnoir_tracker_sm_types.h"
#include "ui_FTNoIR_SM_controls.h"

#include <QMessageBox>
#include <QSettings>
#include <QProcess>
#include "Windows.h"
#include "math.h"

using namespace std;

class FTNoIR_Tracker : public ITracker
{
public:
	FTNoIR_Tracker();
	~FTNoIR_Tracker();

	void Release();
    void Initialize( QFrame *videoframe );
    void StartTracker( HWND parent_window );
    void StopTracker( bool exit );
	bool GiveHeadPoseData(THeadPoseData *data);				// Returns true if confidence is good
	void loadSettings();
	bool SMCreateMapping();

	void getFullName(QString *strToBeFilled);
	void getShortName(QString *strToBeFilled);
	void getDescription(QString *strToBeFilled);

private:
	//
	// global variables
	//
	HANDLE hSMMemMap;
	SMMemMap *pMemData;
	HANDLE hSMMutex;
	QProcess *faceAPI;

	QString trackerFullName;								// Trackers' name and description
	QString trackerShortName;
	QString trackerDescription;

};

// Widget that has controls for SMoIR protocol client-settings.
class TrackerControls: public QWidget, Ui::UICSMClientControls, public ITrackerDialog
{
    Q_OBJECT
public:

	explicit TrackerControls();
    virtual ~TrackerControls();
	void showEvent ( QShowEvent * event );

	void Release();											// Member functions which are accessible from outside the DLL
    void Initialize(QWidget *parent);

	void getFullName(QString *strToBeFilled);
	void getShortName(QString *strToBeFilled);
	void getDescription(QString *strToBeFilled);
	void getIcon(QIcon *icon);

private:
	Ui::UICSMClientControls ui;
	void loadSettings();
	void save();
	bool SMCreateMapping();
	void doCommand( int command );
	void doCommand( int command, int value );

	/** helper **/
	bool settingsDirty;
	int prev_state;											// Previous engine state

	//
	// global variables
	//
	HANDLE hSMMemMap;
	SMMemMap *pMemData;
	HANDLE hSMMutex;
    smEngineHandle *engine_handle;
	QTimer *timUpdateSettings;								// Timer to display current settings

	QString trackerFullName;								// Trackers' name and description
	QString trackerShortName;
	QString trackerDescription;

private slots:
	void doOK();
	void doCancel();
	void settingChanged() { settingsDirty = true; };
	void doTimUpdate();
	void showSettings( int newState );
	void doStartEngine(){
		doCommand(FT_SM_START);
	}
	void doStopEngine(){
		doCommand(FT_SM_STOP);
	}
	void doShowCam(){
		doCommand(FT_SM_SHOW_CAM);
	}
	void doSetFilter(int value){
		doCommand(FT_SM_SET_PAR_FILTER, value);
	}

signals:
     void stateChanged(int newState);

};

