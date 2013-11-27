/* code by Furax49, awaiting copyright information */

#include "ftnoir_tracker_hat.h"
#include "facetracknoir/global-settings.h"
#include <QMessageBox>
#include <QDebug>

FTNoIR_Tracker::FTNoIR_Tracker() :
    SerialPort(nullptr),
    stop(false)
{
	TrackerSettings settings;
	settings.load_ini();
	applysettings(settings);

    //ListErrInf  = new QList<QString>();

	datagram.reserve(30);
    qDebug() << "FTNoIR_Tracker::Initialize() Open SerialPort";
    SerialPort = new QSerialPort(sSerialPortName);
    if (SerialPort->open(QIODevice::ReadWrite | QIODevice::Unbuffered  ) == true) {
        SerialPort->flush();
        SerialPort->setBaudRate(115200);
        SerialPort->setParity(QSerialPort::NoParity);
        SerialPort->setDataBits(QSerialPort::Data8);
        SerialPort->setStopBits(QSerialPort::OneStop);
        SerialPort->setFlowControl(QSerialPort::NoFlowControl);
        //SerialPort->setTimeout(waitTimeout);
        //SerialPort->setQueryMode(QextSerialPort::EventDriven); //Polling
        SerialPort->putChar('S');
    }
    else {
        QMessageBox::warning(0,"FaceTrackNoIR Error", "Unable to open SerialPort",QMessageBox::Ok,QMessageBox::NoButton);
        delete SerialPort;
        SerialPort = NULL;
    }
}

FTNoIR_Tracker::~FTNoIR_Tracker()
{
    stop = true;
    wait();
	if (SerialPort!=NULL) {
		if (SerialPort->isOpen() ) {
			SerialPort->putChar('s'); //Send STOP to Arduino
			SerialPort->close();
		}
		delete SerialPort;
		SerialPort=NULL;
	}

}

//send CENTER to Arduino
void FTNoIR_Tracker::notifyCenter() {
	if (SerialPort!=NULL) {
		if (SerialPort->isOpen() ) {
			SerialPort->putChar('C');
		}
	}
}



//send CENTER to Arduino
void FTNoIR_Tracker::center() {
	if (SerialPort!=NULL) {
		if (SerialPort->isOpen() ) {
			SerialPort->putChar('C');
		}
	}
}

//send RESET to Arduino
void FTNoIR_Tracker::reset() {
	if (SerialPort!=NULL) {
		if (SerialPort->isOpen() ) {
			SerialPort->putChar('R');
		}
	}
}

//send command  to Arduino
void FTNoIR_Tracker::sendcmd(QString* cmd) {
    QMutexLocker locker(&lock);
	QByteArray bytes;
	if (SerialPort!=NULL) {
		if (SerialPort->isOpen() ) {
            bytes.append(cmd->toLatin1());
            SerialPort->write(bytes);
		}
	}
}

// return FPS and last status
void FTNoIR_Tracker::get_info(QString*, int* tps ){
    QMutexLocker locker(&lock);
	*tps=HAT.Code;
#if 0
	if (ListErrInf->size()>0)  {
		*info=ListErrInf->takeFirst();
	} else {
		*info= QString();
	}
#endif
}


/** QThread run @override **/
void FTNoIR_Tracker::run() {
    if (!SerialPort)
        return;
    while (!stop)
    {
        if (SerialPort->bytesAvailable()>=30)
        {
            QMutexLocker locker(&lock);
			datagram.clear();
			datagram=SerialPort->read(30);
            QDataStream datastream(datagram);
            datastream >> ArduinoData;
            if (ArduinoData.Begin==0xAAAA && ArduinoData.End==0x5555 )
            {
                if (ArduinoData.Code <= 1000)
                {
					HAT=ArduinoData;
                }
			} else { 
				SerialPort->read(1);
			}
		}
		usleep(10000);
	}
}

void FTNoIR_Tracker::StartTracker( QFrame* )
{
	start( QThread::TimeCriticalPriority );
	return;
}

bool FTNoIR_Tracker::GiveHeadPoseData(double* data)
{
    QMutexLocker locker(&lock);

    const bool inversions[] = {
        bInvertX, bInvertY, bInvertZ, bInvertYaw, bInvertPitch, bInvertRoll
    };

    const bool enablement[] = {
        bEnableX, bEnableY, bEnableZ, bEnableYaw, bEnablePitch, bEnableRoll
    };

    const int axes[] = {
        iXAxis, iYAxis, iZAxis, iYawAxis, iPitchAxis, iRollAxis
    };

    for (int i = 0; i < 6; i++)
    {
        if (enablement[i])
            data[i] = HAT.Gyro[axes[i]] * (inversions[i] ? -1 : 1);
    }

    return true;
}

void FTNoIR_Tracker::applysettings(const TrackerSettings& settings){
	qDebug()<<"Tracker:: Applying settings";

    QMutexLocker locker(&lock);
	sSerialPortName= settings.SerialPortName;

	bEnableRoll = settings.EnableRoll;
	bEnablePitch = settings.EnablePitch;
	bEnableYaw = settings.EnableYaw;
	bEnableX = settings.EnableX;
	bEnableY = settings.EnableY;
	bEnableZ = settings.EnableZ;

	bInvertRoll = settings.InvertRoll;
	bInvertPitch = settings.InvertPitch;
	bInvertYaw = settings.InvertYaw;
	bInvertX = settings.InvertX;
	bInvertY = settings.InvertY;
	bInvertZ = settings.InvertZ;


    iRollAxis= settings.RollAxis;
    iPitchAxis= settings.PitchAxis;
    iYawAxis= settings.YawAxis;
    iXAxis= settings.XAxis;
    iYAxis= settings.YAxis;
    iZAxis= settings.ZAxis;
}

extern "C" FTNOIR_TRACKER_BASE_EXPORT ITracker* CALLING_CONVENTION GetConstructor()
{
	return new FTNoIR_Tracker;
}
