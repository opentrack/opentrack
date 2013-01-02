#include "..\ftnoir_tracker_base\ftnoir_tracker_base.h"
#include "face-detect.h"
#include "ui_FTNoIR_FD_controls.h"

#include <Qt>
#include <QtCore/QEvent>
#include <Qt/qt_windows.h>

#include <QMessageBox>
#include <QSettings>
#include <QProcess>
#include "math.h"
#include <Windows.h>

using namespace std;

static LPTSTR prog_cmdline = (LPTSTR) TEXT("face-detect.exe");
static LPTSTR fd_shm_name = (LPTSTR) TEXT("face-detect-shm");
static LPTSTR fd_mutex_name = (LPTSTR) TEXT("face-detect-mutex");

class VideoWidget : public QWidget
{
	Q_OBJECT
public:
	VideoWidget(HANDLE hMutex, unsigned char* data, struct face_detect_shm* shm);
protected:
	void paintEvent(QPaintEvent*);
private:
	HANDLE hMutex;
	unsigned char* data;
	struct face_detect_shm* shm;
};

class FTNoIR_Tracker : public ITracker
{
public:
	FTNoIR_Tracker();
	~FTNoIR_Tracker();

    void Initialize( QFrame *videoframe );
    void StartTracker( HWND parent_window );
    void StopTracker( bool exit );
	bool GiveHeadPoseData(THeadPoseData *data);				// Returns true if confidence is good

	void loadSettings();
//	bool setParameterValue(const int index, const float newvalue);
	bool notifyZeroed();
	void refreshVideo();

private:
	bool activep;
	//QList<std::pair<float,float>>	parameterRange;
	//QList<float>					parameterValueAsFloat;
	void TerminateTracker();
	HANDLE hMutex, hMapFile;
	struct face_detect_shm* shm;
	PROCESS_INFORMATION procInfo;
	VideoWidget* ctrl;
	QFrame* qframe;
};

class TrackerControls: public QWidget, Ui::UICFDClientControls, public ITrackerDialog
{
    Q_OBJECT
public:

	explicit TrackerControls();
    virtual ~TrackerControls();
	void Release();											// Member functions which are accessible from outside the DLL
    void Initialize(QWidget *parent);
	void registerTracker(ITracker *tracker) {};
	void unRegisterTracker() {};
	void NotifyZeroing();

private:
	Ui::UICFDClientControls ui;
	void loadSettings();
	void save();

	bool settingsDirty;
	HANDLE hMapFile, hMutex;
	struct face_detect_shm* shm;

private slots:
	void doOK();
	void doCancel();
	void settingChanged() { settingsDirty = true; };
	void doSetRedetectMs(int val);
	void doSetCameraId(int val);
	void doSetVideoWidget(bool val);
signals:
};

//*******************************************************************************************************
// FaceTrackNoIR Tracker DLL. Functions used to get general info on the Tracker
//*******************************************************************************************************
class FTNoIR_TrackerDll : public ITrackerDll
{
public:
	FTNoIR_TrackerDll();
	~FTNoIR_TrackerDll();

    void Initialize();

	void getFullName(QString *strToBeFilled);
	void getShortName(QString *strToBeFilled);
	void getDescription(QString *strToBeFilled);
	void getIcon(QIcon *icon);

private:
	QString trackerFullName;									// Trackers' name and description
	QString trackerShortName;
	QString trackerDescription;
};
