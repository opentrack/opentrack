#include "..\ftnoir_tracker_base\ftnoir_tracker_base.h"
#include "..\ftnoir_tracker_base\ftnoir_tracker_sm_types.h"
#include "ui_FTNoIR_SM_controls.h"

#include <QMessageBox>
#include <QSettings>
#include <QProcess>
#include "Windows.h"
#include "math.h"

using namespace std;

class FTNoIR_Tracker_SM : public ITracker
{
public:
	FTNoIR_Tracker_SM();
	~FTNoIR_Tracker_SM();

	void Release();
    void Initialize( QFrame *videoframe );
    void StartTracker( HWND parent_window );
    void StopTracker( bool exit );
	bool GiveHeadPoseData(THeadPoseData *data);				// Returns true if confidence is good
	void loadSettings();
	bool SMCreateMapping();

	bool setParameterValue(const int index, const float newvalue);

private:
	//
	// global variables
	//
	HANDLE hSMMemMap;
	SMMemMap *pMemData;
	HANDLE hSMMutex;
//	smEngineHeadPoseData new_head_pose;
	QProcess *faceAPI;

	enum
	{
		kPortAddress=0,										// Index in QList
		kNumFilterParameters								// Indicate number of parameters used
	};
	QList<std::pair<float,float>>	parameterRange;
	QList<float>					parameterValueAsFloat;

};

// Widget that has controls for SMoIR protocol client-settings.
class SMClientControls: public QWidget, Ui::UICSMClientControls, public ITrackerDialog
{
    Q_OBJECT
public:

	explicit SMClientControls();
    virtual ~SMClientControls();
	void showEvent ( QShowEvent * event );

	void Release();											// Member functions which are accessible from outside the DLL
    void Initialize(QWidget *parent);

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

