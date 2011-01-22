#include "ftnoir_tracker_base.h"
#include <QThread>
#include <QUdpSocket>
#include "Windows.h"

class FTNoIR_Tracker_UDP : public ITracker, QThread
{
public:
	FTNoIR_Tracker_UDP();
	~FTNoIR_Tracker_UDP();

	int Foo(int n);
    void Release();
    void Initialize();
    void StartTracker();
	void GiveHeadPoseData(THeadPoseData *data);

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
};

FTNoIR_Tracker_UDP::FTNoIR_Tracker_UDP()
{
	inSocket = 0;
	outSocket = 0;

	// Create events
	m_StopThread = CreateEvent(0, TRUE, FALSE, 0);
	m_WaitThread = CreateEvent(0, TRUE, FALSE, 0);

	newHeadPose.x = 1.0f;
	newHeadPose.y = 2.0f;
	newHeadPose.z = 3.0f;
	newHeadPose.yaw   = 4.0f;
	newHeadPose.pitch = 5.0f;
	newHeadPose.roll  = 6.0f;
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
		inSocket->disconnectFromHost();
		inSocket->waitForDisconnected();
		delete inSocket;
	}

	if (outSocket) {
		outSocket->disconnectFromHost();
		outSocket->waitForDisconnected();
		delete outSocket;
	}
}

/** QThread run @override **/
void FTNoIR_Tracker_UDP::run() {

int no_bytes;
QHostAddress sender;
quint16 senderPort;

	//
	// Create UDP-sockets if they don't exist already.
	// They must be created here, because they must be in the new thread (FTNoIR_Tracker_UDP::run())
	//
	if (inSocket == 0) {
		qDebug() << "FTNoIR_Tracker_UDP::run() creating insocket";
		inSocket = new QUdpSocket();
		// Connect the inSocket to the port, to receive messages
		inSocket->bind(QHostAddress::Any, destPort+1);
	}

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

		while (inSocket->hasPendingDatagrams()) {

			QByteArray datagram;
			datagram.resize(inSocket->pendingDatagramSize());

			inSocket->readDatagram( (char * ) &newHeadPose, sizeof(newHeadPose), &sender, &senderPort);
		}

		//for lower cpu load 
		usleep(5000);
		yieldCurrentThread(); 

	}
}

int FTNoIR_Tracker_UDP::Foo(int n)
{
    return n * n;
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

	newHeadPose.x += 1.0f;
	newHeadPose.y += 2.0f;
	newHeadPose.z += 3.0f;
	newHeadPose.yaw   += 4.0f;
	newHeadPose.pitch += 5.0f;
	newHeadPose.roll  += 6.0f;

	data->x = newHeadPose.x;
	data->y = newHeadPose.y;
	data->z = newHeadPose.z;
	data->yaw = newHeadPose.yaw;
	data->pitch = newHeadPose.pitch;
	data->roll = newHeadPose.roll;
	return;
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

