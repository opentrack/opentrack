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


#include "ftnoir_tracker_hat.h"
#include <QMessageBox>
#include <QDebug>
#include <QCoreApplication>

FTNoIR_Tracker::FTNoIR_Tracker()
{
	SerialPort = NULL;
	waitTimeout = 1000;
	TrackerSettings settings;
	settings.load_ini();
	applysettings(settings);


	// Create events
	m_StopThread = CreateEvent(0, TRUE, FALSE, 0);
	m_WaitThread = CreateEvent(0, TRUE, FALSE, 0);

	ListErrInf  = new QList<QString>();

	// prepare & reserve QByteArray
	datagram.reserve(30);
}

FTNoIR_Tracker::~FTNoIR_Tracker()
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
	QReadLocker locker(&rwlock);
	QByteArray bytes;
	if (SerialPort!=NULL) {
		if (SerialPort->isOpen() ) {
			bytes.append(cmd->toAscii());
			SerialPort->write(bytes);
		}
	}
}

// return FPS and last status
void FTNoIR_Tracker::get_info(QString* info, int* tps ){
	QReadLocker locker(&rwlock);
	*tps=HAT.Code;
	if (ListErrInf->size()>0)  {
		*info=ListErrInf->takeFirst();
	} else {
		*info= QString();
	}
}


/** QThread run @override **/
void FTNoIR_Tracker::run() {
	//
	// Read the data that was received.
	//
	forever {

		// Check event for stop thread
		if(::WaitForSingleObject(m_StopThread, 0) == WAIT_OBJECT_0)
		{
			// Set event
			::SetEvent(m_WaitThread);
			qDebug() << "FTNoIR_Tracker::run() terminated run()";
			return;
		}
		if (SerialPort->bytesAvailable()>=30) {
			QWriteLocker locker(&rwlock);
			datagram.clear();
			datagram=SerialPort->read(30);
			QDataStream  datastream(datagram);
			datastream>>ArduinoData;

			if (ArduinoData.Begin==0xAAAA && ArduinoData.End==0x5555 ) {
				if (ArduinoData.Code <= 1000) {
					HAT=ArduinoData;
				} else {
					ListErrInf->push_back(QString::fromAscii(datagram.mid(4,24),24))  ;
				}
			} else { 
				SerialPort->read(1);
			}
		}
		//for lower cpu load 
		usleep(10000);
	}
}

void FTNoIR_Tracker::Initialize( QFrame *videoframe )
{
	qDebug() << "FTNoIR_Tracker::Initialize says: Starting ";

	//
	// Create SerialPort if they don't exist already.
	// They must be created here, because they must be in the new thread (FTNoIR_Tracker::run())
	//

	if (SerialPort == NULL) {
		qDebug() << "FTNoIR_Tracker::Initialize() Open SerialPort";
		SerialPort = new QextSerialPort(sSerialPortName); 
		if (SerialPort->open(QIODevice::ReadWrite | QIODevice::Unbuffered  ) == true) { 
			SerialPort->flush();
			SerialPort->setBaudRate(BAUD115200);  
			SerialPort->setParity(PAR_NONE);  
			SerialPort->setDataBits(DATA_8);  
			SerialPort->setStopBits(STOP_1);  
			SerialPort->setFlowControl(FLOW_OFF);  
			SerialPort->setTimeout(waitTimeout);  
			SerialPort->setQueryMode(QextSerialPort::EventDriven); //Polling  
			// Send START to arduino
			SerialPort->putChar('S');
		}
		else {
			QMessageBox::warning(0,"FaceTrackNoIR Error", "Unable to open SerialPort",QMessageBox::Ok,QMessageBox::NoButton);
			delete SerialPort;
			SerialPort = NULL;
		}
	}
	return;
}

void FTNoIR_Tracker::StartTracker( HWND parent_window )
{
	start( QThread::TimeCriticalPriority );
	return;
}

void FTNoIR_Tracker::StopTracker( bool exit )
{
	//
	// OK, the thread is not stopped, doing this. That might be dangerous anyway...
	//
	if (exit || !exit) return;
	return;
}

//
// Return 6DOF info
//
bool FTNoIR_Tracker::GiveHeadPoseData(THeadPoseData *data)
{
	QReadLocker locker(&rwlock);
	data->frame_number =  HAT.Code;

	if (bEnableYaw) {
		if (bInvertYaw )	data->yaw =  HAT.Gyro[iYawAxe] *  -1.0f;
		else 	data->yaw =  HAT.Gyro[iYawAxe];
	}	

	if (bEnablePitch) {
		if (bInvertPitch)data->pitch =  HAT.Gyro[iPitchAxe] *  -1.0f;
		else data->pitch =  HAT.Gyro[iPitchAxe];
	}

	if (bEnableRoll) {
		if (bInvertRoll) data->roll =  HAT.Gyro[iRollAxe] *  -1.0f; 
		else data->roll =  HAT.Gyro[iRollAxe];
	}

	if (bEnableX) {
		if (bInvertX) data->x =  HAT.Acc[iXAxe]*  -1.0f;
		else data->x =  HAT.Acc[iXAxe];
	}

	if (bEnableY) {
		if (bInvertY) data->y = HAT.Acc[iYAxe]*  -1.0f;
		else data->y =  HAT.Acc[iYAxe];
	}

	if (bEnableZ) {
		if (bInvertZ)  data->z =  HAT.Acc[iZAxe]*  -1.0f;
		else data->z =  HAT.Acc[iZAxe];
	}

	return true;
}



//
// Apply modification Settings 
//
void FTNoIR_Tracker::applysettings(const TrackerSettings& settings){
	qDebug()<<"Tracker:: Applying settings";

	QReadLocker locker(&rwlock);
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


	iRollAxe= settings.RollAxe;
	iPitchAxe= settings.PitchAxe;
	iYawAxe= settings.YawAxe;
	iXAxe= settings.XAxe;
	iYAxe= settings.YAxe;
	iZAxe= settings.ZAxe;
}




////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker object.

// Export both decorated and undecorated names.
//   GetTracker     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetTracker@0  - Common name decoration for __stdcall functions in C language.
#pragma comment(linker, "/export:GetTracker=_GetTracker@0")

FTNOIR_TRACKER_BASE_EXPORT ITrackerPtr __stdcall GetTracker()
{
	return new FTNoIR_Tracker;
}
