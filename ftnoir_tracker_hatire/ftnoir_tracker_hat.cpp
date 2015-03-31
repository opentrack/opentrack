/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2012	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
* Homepage:			http://facetracknoir.sourceforge.net/home/default.htm		*
*																				*
* Copyright (C) 2012	FuraX49 (HAT Tracker plugins)	    	     			*
* Homepage:			http://hatire.sourceforge.net								*
*																				*
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

#include <QDebug>
#include "ftnoir_tracker_hat.h"

FTNoIR_Tracker::FTNoIR_Tracker()
{
    qDebug()<<"Tracker::HAT";

	ComPort =   NULL;

	HAT.Rot[0]=0;
	HAT.Rot[1]=0;
	HAT.Rot[2]=0;
	HAT.Trans[0]=0;
	HAT.Trans[1]=0;
	HAT.Trans[2]=0;


	// prepare & reserve QByteArray
	dataRead.resize(4096);
	dataRead.clear();
	Begin.append((char) 0xAA);
	Begin.append((char) 0xAA);
	End.append((char) 0x55);
	End.append((char) 0x55);

	settings.load_ini();
}

FTNoIR_Tracker::~FTNoIR_Tracker()
{
    qDebug()<<"Tracker::~HAT";
    if (ComPort!=NULL) {
		if (ComPort->isOpen() ) {

#ifdef OPENTRACK_API
            QByteArray Msg;
            ComPort->write(sCmdStop);
            if (!ComPort->waitForBytesWritten(1000)) {
                emit sendMsgInfo("TimeOut in writing CMD");
            } else  {
                Msg.append("\r\n");
                Msg.append("SEND '");
                Msg.append(sCmdStop);
                Msg.append("'\r\n");
            }
            emit sendMsgInfo(Msg);
#endif
			ComPort->close();
            disconnect(ComPort,SIGNAL(readyRead()),0,0);

        }
		delete ComPort;
		ComPort=NULL;
	}
}


//send ZERO to Arduino
bool FTNoIR_Tracker::notifyZeroed() {
    qDebug() << " HAT send ZEROed ";
    sendcmd(sCmdZero);
    return true;
}



//send RESET to Arduino
void FTNoIR_Tracker::reset() {
    qDebug()   << " HAT send RESET ";
    sendcmd(sCmdReset);
}


// Info SerialPort
void FTNoIR_Tracker::SerialInfo() {
	QByteArray Msg;
	if (ComPort!=NULL) {
		if (ComPort->isOpen() ) {
			Msg.append("\r\n");
			Msg.append(ComPort->portName());
			Msg.append("\r\n");
			Msg.append("BAUDRATE :");
			Msg.append(QString::number(ComPort->baudRate()));
			Msg.append("\r\n");
			Msg.append("DataBits :");
			Msg.append(QString::number(ComPort->dataBits()));
			Msg.append("\r\n");
			Msg.append("Parity :");
			switch (ComPort->parity()) {
				case 0:  Msg.append("No parity");
					break; 
				case 2:  Msg.append("Even parity");
					break; 
				case 3:  Msg.append("Odd parity");
					break; 
				case 4:  Msg.append("Space parity");
					break; 
				case 5:  Msg.append("Mark parity");
					break; 
				default:  Msg.append("Unknown parity");
					break; 
			}
			Msg.append("\r\n");
			Msg.append("Stop Bits :");
			switch (ComPort->stopBits()) {
				Msg.append(QString::number(ComPort->stopBits()));
				case 1:  Msg.append("1 stop bit.");
					break; 
				case 2:  Msg.append("2 stop bits.");
					break; 
				case 3:  Msg.append("1.5 stop bits.");
					break; 
				default:  Msg.append("Unknown number of stop bit.");
					break; 
			}
			Msg.append("\r\n");
			Msg.append("Flow Control :");
			switch (ComPort->flowControl()) {
				case 0:  Msg.append("No flow control");
					break; 
				case 1:  Msg.append("Hardware flow control (RTS/CTS)");
					break; 
				case 2:  Msg.append("Software flow control (XON/XOFF)");
					break; 
				default:  Msg.append("Unknown flow control");
					break; 
			}
			emit sendMsgInfo(Msg);

		}
	}
}


//send command  to Arduino
void FTNoIR_Tracker::sendcmd(const QByteArray &cmd) {
	QByteArray Msg;
	if (cmd.length()>0) {
		if (ComPort->isOpen() ) 
		{
			ComPort->write(cmd);
			if (!ComPort->waitForBytesWritten(1000)) {
				emit sendMsgInfo("TimeOut in writing CMD");
			} else  {
				Msg.append("\r\n");
				Msg.append("SEND '");
				Msg.append(cmd);
				Msg.append("'\r\n");
			}
			if  ( !ComPort->waitForReadyRead(1000)) {
				emit sendMsgInfo("TimeOut in response to CMD") ;
			} else {
				emit sendMsgInfo(Msg);
			}
		} else {
			emit sendMsgInfo("ComPort not open")  ;
		}
	}
}


// return FPS 
void FTNoIR_Tracker::get_info( int *tps ){
	*tps=frame_cnt;
	frame_cnt=0;
}

void FTNoIR_Tracker::SerialRead()
{
    QMutexLocker lck(&mutex);
	dataRead+=ComPort->readAll();
}

#ifndef OPENTRACK_API
void FTNoIR_Tracker::Initialize( QFrame *videoframe )
{
	CptError=0;
	dataRead.clear();
	frame_cnt=0;

	settings.load_ini();
	applysettings(settings);
	ComPort =  new QSerialPort(this);
	ComPort->setPortName(sSerialPortName); 
	if (ComPort->open(QIODevice::ReadWrite ) == true) { 
		connect(ComPort, SIGNAL(readyRead()), this, SLOT(SerialRead()));
		if (  
			ComPort->setBaudRate((QSerialPort::BaudRate)iBaudRate)
			&& ComPort->setDataBits((QSerialPort::DataBits)iDataBits) 
			&& ComPort->setParity((QSerialPort::Parity)iParity) 
			&& ComPort->setStopBits((QSerialPort::StopBits)iStopBits)  
			&& ComPort->setFlowControl((QSerialPort::FlowControl)iFlowControl)  
			&& ComPort->clear(QSerialPort::AllDirections)
			&& ComPort->setDataErrorPolicy(QSerialPort::IgnorePolicy)
			) {
				// Wait init arduino sequence 
				for (int i = 1; i <=iDelayInit;  i+=50) {
					if (ComPort->waitForReadyRead(50)) break;
				}
				sendcmd(sCmdInit);
				// Wait init MPU sequence 
				for (int i = 1; i <=iDelayStart;  i+=50) {
					if (ComPort->waitForReadyRead(50)) break;
				}

		} else {
			QMessageBox::warning(0,"FaceTrackNoIR Error", ComPort->errorString(),QMessageBox::Ok,QMessageBox::NoButton);
		}
	}
	else {
		QMessageBox::warning(0,"FaceTrackNoIR Error", "Unable to open ComPort",QMessageBox::Ok,QMessageBox::NoButton);
		delete ComPort;
		ComPort = NULL;
	} 
	return;
}



void FTNoIR_Tracker::StartTracker(HWND parent_window)
{
	// Send  START cmd to IMU
	sendcmd(sCmdStart);
	// Wait start MPU sequence 
	for (int i = 1; i <=iDelaySeq;  i+=50) {
		if (ComPort->waitForReadyRead(50)) break;
	}
	return;
}


void FTNoIR_Tracker::StopTracker( bool exit )
{
	QByteArray Msg;
	if (sCmdStop.length()>0) {
		if (ComPort->isOpen() ) 
		{
			ComPort->write(sCmdStop);
			if (!ComPort->waitForBytesWritten(1000)) {
				emit sendMsgInfo("TimeOut in writing CMD");
			} else  {
				Msg.append("\r\n");
				Msg.append("SEND '");
				Msg.append(sCmdStop);
				Msg.append("'\r\n");
			}	
			emit sendMsgInfo(Msg);
		}
	}
	// OK, the thread is not stopped, doing this. That might be dangerous anyway...
	//
	if (exit || !exit) return;
	return;
}
//send CENTER to Arduino
void FTNoIR_Tracker::notifyCenter() {
    sendcmd(sCmdCenter);
}


#else
void FTNoIR_Tracker::StartTracker(QFrame*)
{
	CptError=0;
	dataRead.clear();
	frame_cnt=0;
    new_frame=false;
	settings.load_ini();
	applysettings(settings);
	ComPort =  new QSerialPort(this);
	ComPort->setPortName(sSerialPortName); 
	if (ComPort->open(QIODevice::ReadWrite ) == true) { 
		connect(ComPort, SIGNAL(readyRead()), this, SLOT(SerialRead()));
		if (  
			ComPort->setBaudRate((QSerialPort::BaudRate)iBaudRate)
			&& ComPort->setDataBits((QSerialPort::DataBits)iDataBits) 
			&& ComPort->setParity((QSerialPort::Parity)iParity) 
			&& ComPort->setStopBits((QSerialPort::StopBits)iStopBits)  
			&& ComPort->setFlowControl((QSerialPort::FlowControl)iFlowControl)  
			&& ComPort->clear(QSerialPort::AllDirections)
			&& ComPort->setDataErrorPolicy(QSerialPort::IgnorePolicy)
			) {
                qDebug()  << QTime::currentTime()  << " HAT OPEN   on " << ComPort->portName() <<  ComPort->baudRate() <<  ComPort->dataBits() <<  ComPort->parity() <<  ComPort->stopBits() <<  ComPort->flowControl();

				// Wait init arduino sequence 
				for (int i = 1; i <=iDelayInit;  i+=50) {
					if (ComPort->waitForReadyRead(50)) break;
				}
                qDebug()  << QTime::currentTime()  << " HAT send INIT ";
				sendcmd(sCmdInit);
				// Wait init MPU sequence 
				for (int i = 1; i <=iDelayStart;  i+=50) {
					if (ComPort->waitForReadyRead(50)) break;
				}
				// Send  START cmd to IMU
                qDebug()  << QTime::currentTime()  << " HAT send START ";
                sendcmd(sCmdStart);

				// Wait start MPU sequence 
				for (int i = 1; i <=iDelaySeq;  i+=50) {
					if (ComPort->waitForReadyRead(50)) break;
				}
                qDebug()  << QTime::currentTime()  << " HAT wait MPU ";
        } else {
			QMessageBox::warning(0,"FaceTrackNoIR Error", ComPort->errorString(),QMessageBox::Ok,QMessageBox::NoButton);
		}
	}
	else {
		QMessageBox::warning(0,"FaceTrackNoIR Error", "Unable to open ComPort",QMessageBox::Ok,QMessageBox::NoButton);
		delete ComPort;
		ComPort = NULL;
	} 
	return;

}

//send CENTER to Arduino
void FTNoIR_Tracker::center() {
    qDebug()   << " HAT send CENTER ";
    sendcmd(sCmdCenter);
}

//Return speed FPS sketch  Arduino
int FTNoIR_Tracker::preferredHz() {
    qDebug()  << " HAT return Preferred FPS " << iFpsArduino;
    return iFpsArduino;
}

#endif


//
// Return 6DOF info
//
#ifdef OPENTRACK_API
void FTNoIR_Tracker::GetHeadPoseData(double *data)
#else
bool FTNoIR_Tracker::GiveHeadPoseData(THeadPoseData *data)
#endif
{
    QMutexLocker lck(&mutex);
	while  (dataRead.length()>=30) {
		if ((dataRead.startsWith(Begin) &&  ( dataRead.mid(28,2)==End )) )  { // .Begin==0xAAAA .End==0x5555
			QDataStream  datastream(dataRead.left(30));
			if (bBigEndian)	datastream.setByteOrder(QDataStream::BigEndian );
			else datastream.setByteOrder(QDataStream::LittleEndian );
			datastream>>ArduinoData;
			frame_cnt++;
			if (ArduinoData.Code <= 1000) {
				HAT=ArduinoData;
                new_frame=true;
			} else {
				emit sendMsgInfo(dataRead.mid(4,24))  ;
			}
			dataRead.remove(0,30);
		} else {
			// resynchro trame 
			int index =	dataRead.indexOf(Begin);
			if (index==-1) {
				index=dataRead.length();
			} 
			emit sendMsgInfo(dataRead.mid(0,index))  ;
			dataRead.remove(0,index);
			CptError++;
		}
	}

	if (CptError>50) {
		emit sendMsgInfo("Can't find HAT frame")  ;
		CptError=0;
#ifndef OPENTRACK_API
		return false;
#endif
	}
    if  (new_frame) {
#ifdef OPENTRACK_API

	if (bEnableYaw) {
        if (bInvertYaw )	data[Yaw] =  HAT.Rot[iYawAxe] *  -1.0f;
        else 	data[Yaw] = HAT.Rot[iYawAxe];
    } else 	data[Yaw] =0;

	if (bEnablePitch) {
        if (bInvertPitch) data[Pitch] =  HAT.Rot[iPitchAxe] *  -1.0f;
        else data[Pitch] =   HAT.Rot[iPitchAxe];
    } else data[Pitch] = 0;

	if (bEnableRoll) {
        if (bInvertRoll) data[Roll] =  HAT.Rot[iRollAxe] *  -1.0f;
        else data[Roll] =  HAT.Rot[iRollAxe];
    } else data[Roll] =0;

	if (bEnableX) {
        if (bInvertX) data[TX] =  HAT.Trans[iXAxe]*  -1.0f;
        else data[TX] =   HAT.Trans[iXAxe];
    } else data[TX] =0;

	if (bEnableY) {
        if (bInvertY) data[TY] =  HAT.Trans[iYAxe]*  -1.0f;
        else data[TY] =  HAT.Trans[iYAxe];
    } else data[TY] =0;

	if (bEnableZ) {
        if (bInvertZ)  data[TZ] =   HAT.Trans[iZAxe]*  -1.0f;
        else data[TZ] =   HAT.Trans[iZAxe];
    } else data[TZ] =0;
#else

	if (bEnableYaw) {
		if (bInvertYaw )	data->yaw = (double) HAT.Rot[iYawAxe] *  -1.0f;
		else 	data->yaw = (double) HAT.Rot[iYawAxe];
	}	

	if (bEnablePitch) {
		if (bInvertPitch)data->pitch = (double) HAT.Rot[iPitchAxe] *  -1.0f;
		else data->pitch = (double) HAT.Rot[iPitchAxe];
	}

	if (bEnableRoll) {
		if (bInvertRoll) data->roll = (double) HAT.Rot[iRollAxe] *  -1.0f; 
		else data->roll = (double) HAT.Rot[iRollAxe];
	}

	if (bEnableX) {
		if (bInvertX) data->x = (double) HAT.Trans[iXAxe]*  -1.0f;
		else data->x = (double) HAT.Trans[iXAxe];
	}

	if (bEnableY) {
		if (bInvertY) data->y = (double) HAT.Trans[iYAxe]*  -1.0f;
		else data->y = (double) HAT.Trans[iYAxe];
	}

	if (bEnableZ) {
		if (bInvertZ)  data->z = (double) HAT.Trans[iZAxe]*  -1.0f;
		else data->z = (double) HAT.Trans[iZAxe];
	}
    return true;
#endif
     new_frame=false;
	// For debug
	//data->x=dataRead.length();
	//data->y=CptError;
    }
}



//
// Apply modification Settings 
//
void FTNoIR_Tracker::applysettings(const TrackerSettings& settings){
    QMutexLocker lck(&mutex);
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

	iBaudRate=settings.pBaudRate;
	iDataBits=settings.pDataBits;
	iParity=settings.pParity;
	iStopBits=settings.pStopBits;
	iFlowControl=settings.pFlowControl;

	sCmdStart= settings.CmdStart.toLatin1();
	sCmdStop= settings.CmdStop.toLatin1();
	sCmdInit= settings.CmdInit.toLatin1();
	sCmdReset= settings.CmdReset.toLatin1();
	sCmdCenter= settings.CmdCenter.toLatin1();
	sCmdZero= settings.CmdZero.toLatin1();

	iDelayInit=settings.DelayInit;
	iDelayStart=settings.DelayStart;
	iDelaySeq=settings.DelaySeq;

	bBigEndian=settings.BigEndian;
#ifdef OPENTRACK_API
    iFpsArduino=settings.FPSArduino;
#endif
}



////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker object.

// Export both decorated and undecorated names.
//   GetTracker     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetTracker@0  - Common name decoration for __stdcall functions in C language.
////////////////////////////////////////////////////////////////////////////////
#ifdef OPENTRACK_API
extern "C" FTNOIR_TRACKER_BASE_EXPORT ITracker* CALLING_CONVENTION GetConstructor()
#else
#pragma comment(linker, "/export:GetTracker=_GetTracker@0")
FTNOIR_TRACKER_BASE_EXPORT ITrackerPtr __stdcall GetTracker()
#endif
{
	return new FTNoIR_Tracker;
}
