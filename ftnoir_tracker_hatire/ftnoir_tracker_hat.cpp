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

#include "ftnoir_tracker_hat.h"

FTNoIR_Tracker::FTNoIR_Tracker()
{
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
}

FTNoIR_Tracker::~FTNoIR_Tracker()
{
	if (ComPort!=NULL) {
		if (ComPort->isOpen() ) {
			ComPort->close();
		}
		delete ComPort;
		ComPort=NULL;
	}
}

//send CENTER to Arduino
void FTNoIR_Tracker::notifyCenter() {
    sendcmd(static_cast<QString>(settings.CmdCenter).toLatin1());
}

//send ZERO to Arduino
bool FTNoIR_Tracker::notifyZeroed() {
    sendcmd(static_cast<QString>(settings.CmdZero).toLatin1());
	return true;
}

//send RESET to Arduino
void FTNoIR_Tracker::reset() {
        sendcmd(static_cast<QString>(settings.CmdReset).toLatin1());
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

#else
void FTNoIR_Tracker::StartTracker(QFrame*)
{
    static const int databits_lookup[] = {
        QSerialPort::Data5,
        QSerialPort::Data6,
        QSerialPort::Data7,
        QSerialPort::Data8,
        QSerialPort::UnknownDataBits
    };

    struct Local {
        static int idx(int max, int value)
        {
            if (value < 0)
                return 0;
            if (max > value)
                return value;
            return max - 1;
        }
    };

    static const int parity_lookup[] = {
        QSerialPort::NoParity,
        QSerialPort::EvenParity,
        QSerialPort::OddParity,
        QSerialPort::SpaceParity,
        QSerialPort::MarkParity,
        QSerialPort::UnknownParity
    };

    static const int stopbits_lookup[] = {
        QSerialPort::OneStop,
        QSerialPort::OneAndHalfStop,
        QSerialPort::TwoStop,
        QSerialPort::UnknownStopBits
    };

    static const int flowctl_lookup[] = {
        QSerialPort::NoFlowControl,
        QSerialPort::HardwareControl,
        QSerialPort::SoftwareControl,
    };

    static const int baudrate_lookup[] = {
        QSerialPort::Baud1200,
        QSerialPort::Baud2400,
        QSerialPort::Baud4800,
        QSerialPort::Baud9600,
        QSerialPort::Baud19200,
        QSerialPort::Baud38400,
        QSerialPort::Baud57600,
        QSerialPort::Baud115200,
        QSerialPort::UnknownBaud
    };

	CptError=0;
	dataRead.clear();
	frame_cnt=0;
	ComPort =  new QSerialPort(this);
    {
        ComPort->setPortName(QSerialPortInfo::availablePorts().value(settings.SerialPortName).portName());
    }
    if (ComPort->open(QIODevice::ReadWrite ) == true) {
		connect(ComPort, SIGNAL(readyRead()), this, SLOT(SerialRead()));
		if (  
            ComPort->setBaudRate(baudrate_lookup[Local::idx(8, settings.pBaudRate)])
            && ComPort->setDataBits((QSerialPort::DataBits)databits_lookup[Local::idx(4, settings.pDataBits)])
            && ComPort->setParity((QSerialPort::Parity)parity_lookup[Local::idx(5, settings.pParity)])
            && ComPort->setStopBits((QSerialPort::StopBits)stopbits_lookup[Local::idx(3, settings.pStopBits)])
            && ComPort->setFlowControl((QSerialPort::FlowControl)flowctl_lookup[Local::idx(3, settings.pFlowControl)])
			&& ComPort->clear(QSerialPort::AllDirections)
            && ComPort->setDataErrorPolicy(QSerialPort::IgnorePolicy)
        ){
				// Wait init arduino sequence 
                for (int i = 1; i <=settings.DelayInit;  i+=50) {
					if (ComPort->waitForReadyRead(50)) break;
				}
                sendcmd(static_cast<QString>(settings.CmdInit).toLatin1());
				// Wait init MPU sequence 
                for (int i = 1; i <=settings.DelayStart;  i+=50) {
					if (ComPort->waitForReadyRead(50)) break;
				}
				// Send  START cmd to IMU
                sendcmd(static_cast<QString>(settings.CmdStart).toLatin1());

				// Wait start MPU sequence 
                for (int i = 1; i <=settings.DelaySeq;  i+=50) {
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
#endif


//
// Return 6DOF info
//
#ifdef OPENTRACK_API
#define THeadPoseData double
#endif

void FTNoIR_Tracker::GetHeadPoseData(THeadPoseData *data)
{
    QMutexLocker lck(&mutex);
	while  (dataRead.length()>=30) {
		if ((dataRead.startsWith(Begin) &&  ( dataRead.mid(28,2)==End )) )  { // .Begin==0xAAAA .End==0x5555
			QDataStream  datastream(dataRead.left(30));
            if (settings.BigEndian)	datastream.setByteOrder(QDataStream::BigEndian );
			else datastream.setByteOrder(QDataStream::LittleEndian );
			datastream>>ArduinoData;
			frame_cnt++;
			if (ArduinoData.Code <= 1000) {
				HAT=ArduinoData;
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
        return;
	}
	data[frame_cnt] = (long) HAT.Code;

    struct Fun {
        static int clamp3(int foo)
        {
            if (foo > 2)
                return 2;
            if (foo < 0)
                return 0;
            return foo;
        }
    };

    if (settings.EnableYaw) {
        if (settings.InvertYaw)	data[Yaw] = (double)  HAT.Rot[Fun::clamp3(settings.YawAxe)] *  -1.0f;
        else 	data[Yaw] = (double) HAT.Rot[Fun::clamp3(settings.YawAxe)];
	}	

    if (settings.EnablePitch) {
        if (settings.InvertPitch) data[Pitch] = (double) HAT.Rot[Fun::clamp3(settings.PitchAxe)] *  -1.0f;
        else data[Pitch] = (double) HAT.Rot[Fun::clamp3(settings.InvertPitch)];
	}

    if (settings.EnableRoll) {
        if (settings.InvertRoll) data[Roll] = (double) HAT.Rot[Fun::clamp3(settings.RollAxe)] *  -1.0f;
        else data[Roll] = (double) HAT.Rot[Fun::clamp3(settings.RollAxe)];
	}

    if (settings.EnableX) {
        if (settings.InvertX) data[TX] =(double)  HAT.Trans[Fun::clamp3(settings.XAxe)]*  -1.0f;
        else data[TX] =  HAT.Trans[Fun::clamp3(settings.XAxe)];
	}

    if (settings.EnableY) {
        if (settings.InvertY) data[TY] =(double) HAT.Trans[Fun::clamp3(settings.YAxe)]*  -1.0f;
        else data[TY] =  HAT.Trans[Fun::clamp3(settings.YAxe)];
	}

    if (settings.EnableZ) {
        if (settings.InvertZ)  data[TZ] =  HAT.Trans[Fun::clamp3(settings.ZAxe)]*  -1.0f;
        else data[TZ] =  HAT.Trans[Fun::clamp3(settings.ZAxe)];
	}
}

void FTNoIR_Tracker::applysettings(const TrackerSettings& settings){
    QMutexLocker lck(&mutex);
    settings.b->reload();
}

#ifdef OPENTRACK_API
extern "C" FTNOIR_TRACKER_BASE_EXPORT ITracker* CALLING_CONVENTION GetConstructor()
#else
#pragma comment(linker, "/export:GetTracker=_GetTracker@0")
FTNOIR_TRACKER_BASE_EXPORT ITrackerPtr __stdcall GetTracker()
#endif
{
	return new FTNoIR_Tracker;
}
