#include "..\ftnoir_tracker_base\ftnoir_tracker_base.h"
#include "..\ftnoir_tracker_base\ftnoir_tracker_sm_types.h"
#include "ui_FTNoIR_SMClientcontrols.h"

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
    void StopTracker();
	bool GiveHeadPoseData(THeadPoseData *data);				// Returns true if confidence is good
	void loadSettings();
	bool SMCreateMapping();

	bool setParameterValue(const int index, const float newvalue);

private:
	/** face api variables **/
	//APIScope *faceapi_scope;
 //   QSharedPointer<EngineBase> _engine;
	//VideoDisplayWidget *_display;
	//QVBoxLayout *l;
	//MainWindow *main_window;
	//parameter list for the filter-function(s)

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
