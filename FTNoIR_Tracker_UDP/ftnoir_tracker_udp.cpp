#include "ftnoir_tracker_base.h"
#include <QThread>
#include <QUdpSocket>
#include <QMessageBox>
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
	void GiveHeadPoseData(THeadPoseData *data);

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

	//
	// Create UDP-sockets if they don't exist already.
	// They must be created here, because they must be in the new thread (FTNoIR_Tracker_UDP::run())
	//
	if (inSocket == 0) {
		qDebug() << "FTNoIR_Tracker_UDP::run() creating insocket";
		inSocket = new QUdpSocket();
		// Connect the inSocket to the port, to receive messages
		
		if (!inSocket->bind(QHostAddress::Any, (int) parameterValueAsFloat[kPortAddress], QUdpSocket::ShareAddress )) {
			QMessageBox::warning(0,"FaceTrackNoIR Error", "Unable to bind UDP-port",QMessageBox::Ok,QMessageBox::NoButton);
			delete inSocket;
			inSocket = 0;
		}
	}

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
		yieldCurrentThread(); 
	}
}

void FTNoIR_Tracker_UDP::Release()
{
    delete this;
}

void FTNoIR_Tracker_UDP::Initialize()
{
	return;
}

void FTNoIR_Tracker_UDP::StartTracker()
{
	start( QThread::TimeCriticalPriority );
	return;
}

void FTNoIR_Tracker_UDP::GiveHeadPoseData(THeadPoseData *data)
{
	data->x = newHeadPose.x;
	data->y = newHeadPose.y;
	data->z = newHeadPose.z;
	data->yaw = newHeadPose.yaw;
	data->pitch = newHeadPose.pitch;
	data->roll = newHeadPose.roll;
	return;
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

