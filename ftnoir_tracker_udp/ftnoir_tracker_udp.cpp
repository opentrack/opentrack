#include "ftnoir_tracker_udp.h"
#include "facetracknoir/plugin-support.h"

FTNoIR_Tracker::FTNoIR_Tracker() : last_recv_pose { 0,0,0, 0,0,0 }, should_quit(false) {}

FTNoIR_Tracker::~FTNoIR_Tracker()
{
    should_quit = true;
    wait();
}

void FTNoIR_Tracker::run() {
    QByteArray datagram;
    datagram.resize(sizeof(last_recv_pose));
    for (;;) {
        if (should_quit)
            break;
        QMutexLocker foo(&mutex);
        while (sock.hasPendingDatagrams()) {
            sock.readDatagram((char * ) last_recv_pose, sizeof(double[6]));
        }
        msleep(1);
    }
}

void FTNoIR_Tracker::StartTracker(QFrame*)
{
    (void) sock.bind(QHostAddress::Any, (int) s.port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
	start();
}

void FTNoIR_Tracker::GetHeadPoseData(double *data)
{
    QMutexLocker foo(&mutex);
    for (int i = 0; i < 6; i++)
        data[i] = last_recv_pose[i];
}

extern "C" OPENTRACK_EXPORT ITracker* GetConstructor()
{
    return new FTNoIR_Tracker;
}
