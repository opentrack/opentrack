#include "ftnoir_tracker_udp.h"

FTNoIR_Tracker_UDP::FTNoIR_Tracker_UDP()
{
	inSocket = 0;
	outSocket = 0;

	// Create events
	m_StopThread = CreateEvent(0, TRUE, FALSE, 0);
	m_WaitThread = CreateEvent(0, TRUE, FALSE, 0);

	//allocate memory for the parameters
	parameterValueAsFloat.clear();
	parameterRange.clear();

	// Add the parameters to the list
	parameterRange.append(std::pair<float,float>(1000.0f,9999.0f));
	parameterValueAsFloat.append(0.0f);
	setParameterValue(kPortAddress,5551.0f);

	newHeadPose.x = 0.0f;
	newHeadPose.y = 0.0f;
	newHeadPose.z = 0.0f;
	newHeadPose.yaw   = 0.0f;
	newHeadPose.pitch = 0.0f;
	newHeadPose.roll  = 0.0f;
}

FTNoIR_Tracker_UDP::~FTNoIR_Tracker_UDP()
{
	// Trigger thread to stop
	::SetEvent(m_StopThread);

	// Wait until thread finished
	if (isRunning()) {
		::WaitForSingleObject(m_WaitThread, INFINITE);
	}

	// Close handles
	::CloseHandle(m_StopThread);
	::CloseHandle(m_WaitThread);

	if (inSocket) {
		inSocket->close();
		delete inSocket;
	}

	if (outSocket) {
		outSocket->close();
		delete outSocket;
	}
}

/** QThread run @override **/
void FTNoIR_Tracker_UDP::run() {

int no_bytes;
QHostAddress sender;
quint16 senderPort;

	//
	// Read the data that was received.
	//
	forever {

	    // Check event for stop thread
		if(::WaitForSingleObject(m_StopThread, 0) == WAIT_OBJECT_0)
		{
			// Set event
			::SetEvent(m_WaitThread);
			qDebug() << "FTNoIR_Tracker_UDP::run() terminated run()";
			return;
		}

		if (inSocket != 0) {
			while (inSocket->hasPendingDatagrams()) {

				QByteArray datagram;
				datagram.resize(inSocket->pendingDatagramSize());

				inSocket->readDatagram( (char * ) &newHeadPose, sizeof(newHeadPose), &sender, &senderPort);
			}
		}
		else {
			qDebug() << "FTNoIR_Tracker_UDP::run() insocket not ready: exit run()";
			return;
		}

		//for lower cpu load 
		usleep(10000);
//		yieldCurrentThread(); 
	}
}

void FTNoIR_Tracker_UDP::Release()
{
    delete this;
}

void FTNoIR_Tracker_UDP::Initialize( QFrame *videoframe )
{
	qDebug() << "FTNoIR_Tracker_UDP::Initialize says: Starting ";
	loadSettings();

	//
	// Create UDP-sockets if they don't exist already.
	// They must be created here, because they must be in the new thread (FTNoIR_Tracker_UDP::run())
	//
	if (inSocket == 0) {
		qDebug() << "FTNoIR_Tracker_UDP::Initialize() creating insocket";
		inSocket = new QUdpSocket();
		// Connect the inSocket to the port, to receive messages
		
		if (!inSocket->bind(QHostAddress::Any, (int) parameterValueAsFloat[kPortAddress], QUdpSocket::ShareAddress )) {
			QMessageBox::warning(0,"FaceTrackNoIR Error", "Unable to bind UDP-port",QMessageBox::Ok,QMessageBox::NoButton);
			delete inSocket;
			inSocket = 0;
		}
	}

	return;
}

void FTNoIR_Tracker_UDP::StartTracker( HWND parent_window )
{
	start( QThread::TimeCriticalPriority );
	return;
}

void FTNoIR_Tracker_UDP::StopTracker( bool exit )
{
	//
	// OK, the thread is not stopped, doing this. That might be dangerous anyway...
	//
	if (exit || !exit) return;
	return;
}

bool FTNoIR_Tracker_UDP::GiveHeadPoseData(THeadPoseData *data)
{
	data->x = newHeadPose.x;
	data->y = newHeadPose.y;
	data->z = newHeadPose.z;
	data->yaw = newHeadPose.yaw;
	data->pitch = newHeadPose.pitch;
	data->roll = newHeadPose.roll;
	return true;
}

bool FTNoIR_Tracker_UDP::setParameterValue(const int index, const float newvalue)
{
	if ((index >= 0) && (index < parameterValueAsFloat.size()))
	{
		//
		// Limit the new value, using the defined range.
		//
		if (newvalue < parameterRange[index].first) {
			parameterValueAsFloat[index] = parameterRange[index].first;
		}
		else {
			if (newvalue > parameterRange[index].second) {
				parameterValueAsFloat[index] = parameterRange[index].second;
			}
			else {
				parameterValueAsFloat[index] = newvalue;
			}
		}

//		updateParameterString(index);
		return true;
	}
	else
	{
		return false;
	}
};

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Tracker_UDP::loadSettings() {

	qDebug() << "FTNoIR_Tracker_UDP::loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "FTNoIR_Tracker_UDP::loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "FTNClient" );
	setParameterValue(kPortAddress, (float) iniFile.value ( "PortNumber", 5550 ).toInt());
	iniFile.endGroup ();
}


////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker object.

// Export both decorated and undecorated names.
//   GetTracker     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetTracker@0  - Common name decoration for __stdcall functions in C language.
#pragma comment(linker, "/export:GetTracker=_GetTracker@0")

FTNOIR_TRACKER_BASE_EXPORT TRACKERHANDLE __stdcall GetTracker()
{
	return new FTNoIR_Tracker_UDP;
}

//*******************************************************************************************************
// FaceTrackNoIR Client Settings-dialog.
//*******************************************************************************************************

//
// Constructor for server-settings-dialog
//
FTNClientControls::FTNClientControls() :
QWidget()
{
	ui.setupUi( this );

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
	//connect(ui.spinIPFirstNibble, SIGNAL(valueChanged(int)), this, SLOT(settingChanged()));
	//connect(ui.spinIPSecondNibble, SIGNAL(valueChanged(int)), this, SLOT(settingChanged()));
	//connect(ui.spinIPThirdNibble, SIGNAL(valueChanged(int)), this, SLOT(settingChanged()));
	//connect(ui.spinIPFourthNibble, SIGNAL(valueChanged(int)), this, SLOT(settingChanged()));
	connect(ui.spinPortNumber, SIGNAL(valueChanged(int)), this, SLOT(settingChanged()));

	// Load the settings from the current .INI-file
	loadSettings();
}

//
// Destructor for server-dialog
//
FTNClientControls::~FTNClientControls() {
	qDebug() << "~FTNClientControls() says: started";
}

void FTNClientControls::Release()
{
    delete this;
}

//
// Initialize tracker-client-dialog
//
void FTNClientControls::Initialize(QWidget *parent) {

	QPoint offsetpos(100, 100);
	if (parent) {
		this->move(parent->pos() + offsetpos);
	}
	show();
}

//
// OK clicked on server-dialog
//
void FTNClientControls::doOK() {
	save();
	this->close();
}

// override show event
void FTNClientControls::showEvent ( QShowEvent * event ) {
	loadSettings();
}

//
// Cancel clicked on server-dialog
//
void FTNClientControls::doCancel() {
	//
	// Ask if changed Settings should be saved
	//
	if (settingsDirty) {
		int ret = QMessageBox::question ( this, "Settings have changed", "Do you want to save the settings?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Discard );

		qDebug() << "doCancel says: answer =" << ret;

		switch (ret) {
			case QMessageBox::Save:
				save();
				this->close();
				break;
			case QMessageBox::Discard:
				this->close();
				break;
			case QMessageBox::Cancel:
				// Cancel was clicked
				break;
			default:
				// should never be reached
			break;
		}
	}
	else {
		this->close();
	}
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNClientControls::loadSettings() {

//	qDebug() << "loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

//	qDebug() << "loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "FTNClient" );
	//ui.spinIPFirstNibble->setValue( iniFile.value ( "IP-1", 192 ).toInt() );
	//ui.spinIPSecondNibble->setValue( iniFile.value ( "IP-2", 168 ).toInt() );
	//ui.spinIPThirdNibble->setValue( iniFile.value ( "IP-3", 2 ).toInt() );
	//ui.spinIPFourthNibble->setValue( iniFile.value ( "IP-4", 1 ).toInt() );

	ui.spinPortNumber->setValue( iniFile.value ( "PortNumber", 5550 ).toInt() );
	iniFile.endGroup ();

	settingsDirty = false;
}

//
// Save the current Settings to the currently 'active' INI-file.
//
void FTNClientControls::save() {

	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "FTNClient" );
	//iniFile.setValue ( "IP-1", ui.spinIPFirstNibble->value() );
	//iniFile.setValue ( "IP-2", ui.spinIPSecondNibble->value() );
	//iniFile.setValue ( "IP-3", ui.spinIPThirdNibble->value() );
	//iniFile.setValue ( "IP-4", ui.spinIPFourthNibble->value() );
	iniFile.setValue ( "PortNumber", ui.spinPortNumber->value() );
	iniFile.endGroup ();

	settingsDirty = false;
}
////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker-settings dialog object.

// Export both decorated and undecorated names.
//   GetTrackerDialog     - Undecorated name, which can be easily used with GetProcAddress
//                          Win32 API function.
//   _GetTrackerDialog@0  - Common name decoration for __stdcall functions in C language.
#pragma comment(linker, "/export:GetTrackerDialog=_GetTrackerDialog@0")

FTNOIR_TRACKER_BASE_EXPORT TRACKERDIALOGHANDLE __stdcall GetTrackerDialog( )
{
	return new FTNClientControls;
}
