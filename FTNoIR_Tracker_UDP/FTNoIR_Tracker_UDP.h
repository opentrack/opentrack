#include "..\ftnoir_tracker_base\ftnoir_tracker_base.h"
#include "ui_FTNoIR_FTNClientcontrols.h"
#include <QThread>
#include <QUdpSocket>
#include <QMessageBox>
#include <QSettings>
#include "Windows.h"
#include "math.h"

class FTNoIR_Tracker_UDP : public ITracker, QThread
{
public:
	FTNoIR_Tracker_UDP();
	~FTNoIR_Tracker_UDP();

	void Release();
    void Initialize();
    void StartTracker();
    void StopTracker();
	bool GiveHeadPoseData(THeadPoseData *data);
	void loadSettings();

	bool setParameterValue(const int index, const float newvalue);

protected:
	void run();												// qthread override run method

private:
	// Handles to neatly terminate thread...
	HANDLE m_StopThread;
	HANDLE m_WaitThread;

	// UDP socket-variables
	QUdpSocket *inSocket;									// Receive from ...
	QUdpSocket *outSocket;									// Send to ...
	QHostAddress destIP;									// Destination IP-address
	int destPort;											// Destination port-number
	QHostAddress srcIP;										// Source IP-address
	int srcPort;											// Source port-number

	THeadPoseData newHeadPose;								// Structure with new headpose

	//parameter list for the filter-function(s)
	enum
	{
		kPortAddress=0,										// Index in QList
		kNumFilterParameters								// Indicate number of parameters used
	};
	QList<std::pair<float,float>>	parameterRange;
	QList<float>					parameterValueAsFloat;

};

// Widget that has controls for FTNoIR protocol client-settings.
class FTNClientControls: public QWidget, Ui::UICFTNClientControls, public ITrackerDialog
{
    Q_OBJECT
public:

	explicit FTNClientControls( QWidget *parent=0, Qt::WindowFlags f=0 );
    virtual ~FTNClientControls();
	void showEvent ( QShowEvent * event );

	void Release();											// Member functions which are accessible from outside the DLL
    void Initialize(QWidget *parent);

private:
	Ui::UICFTNClientControls ui;
	void loadSettings();
	void save();

	/** helper **/
	bool settingsDirty;

private slots:
	void doOK();
	void doCancel();
	void settingChanged() { settingsDirty = true; };
};


