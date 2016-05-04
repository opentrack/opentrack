/* Homepage         http://facetracknoir.sourceforge.net/home/default.htm        *
 *                                                                               *
 * ISC License (ISC)                                                             *
 *                                                                               *
 * Copyright (c) 2015, Wim Vriend                                                *
 *                                                                               *
 * Permission to use, copy, modify, and/or distribute this software for any      *
 * purpose with or without fee is hereby granted, provided that the above        *
 * copyright notice and this permission notice appear in all copies.             *
 */
#include <QDebug>
#include "ftnoir_tracker_hat.h"

hatire::hatire()
{
	HAT.Rot[0]=0;
	HAT.Rot[1]=0;
	HAT.Rot[2]=0;
	HAT.Trans[0]=0;
	HAT.Trans[1]=0;
	HAT.Trans[2]=0;

	Begin.append((char) 0xAA);
	Begin.append((char) 0xAA);
	End.append((char) 0x55);
	End.append((char) 0x55);
	
	settings.load_ini();
}

hatire::~hatire()
{
}

//send RESET to Arduino
void hatire::reset()
{
    t.sendcmd(ts.sCmdReset);
}

// return FPS 
void hatire::get_info( int *tps )
{
	*tps=frame_cnt;
	frame_cnt=0;
}
void hatire::start_tracker(QFrame*)
{
	CptError=0;
	frame_cnt=0;
    new_frame=false;
	settings.load_ini();
	applysettings(settings);
    t.Log("Starting Tracker");

    serial_result ret = t.init_serial_port();

    switch (ret.code)
    {
    case result_ok:
        break;
    case result_error:
        QMessageBox::warning(0,"Error", ret.error, QMessageBox::Ok,QMessageBox::NoButton);
        break;
    case result_open_error:
        QMessageBox::warning(0,"Error", "Unable to open ComPort: " + ret.error, QMessageBox::Ok,QMessageBox::NoButton);
        break;
    }

    t.start(ts);
}

void hatire::serial_info()
{
    t.serial_info();
}

void hatire::send_serial_command(const QByteArray& x)
{
    t.sendcmd(x);
}

//
// Return 6DOF info
//
void hatire::data(double *data)
{
    {
        QMutexLocker l(&t.data_mtx);

        QByteArray dataRead(t.flush_data_read_nolock());

        while (dataRead.length() >= 30)
        {
            t.Log(dataRead.toHex());
            if (dataRead.startsWith(Begin) && dataRead.mid(28,2) == End)
            { // .Begin==0xAAAA .End==0x5555
                QDataStream datastream(dataRead.left(30));
                if (ts.bBigEndian)	datastream.setByteOrder(QDataStream::BigEndian );
                else datastream.setByteOrder(QDataStream::LittleEndian);
                datastream>>ArduinoData;
                frame_cnt++;
                if (ArduinoData.Code <= 1000)
                {
                    HAT=ArduinoData;
                    new_frame=true;
                }
                else
                {
                    emit t.serial_debug_info(dataRead.mid(4,24))  ;
                }
                dataRead.remove(0,30);
            }
            else
            {
                bool ok = true;
                // resynchro trame
                int index =	dataRead.indexOf(Begin);
                if (index==-1) {
                    ok = false;
                    index=dataRead.length();
                }
                emit t.serial_debug_info(dataRead.mid(0,index))  ;
                dataRead.remove(0,index);
                CptError++;
                qDebug() << QTime::currentTime() << "hatire resync stream" << "index" << index << "ok" << ok;
            }
        }

        t.prepend_unread_data_nolock(dataRead);
    }

    if (CptError > 50)
    {
        emit t.serial_debug_info("Can't find HAT frame");
		CptError=0;
	}

    // XXX fix copy-pasted code -sh 20160410

	// Need to handle this differently in opentrack as opposed to tracknoir
    //if  (new_frame) { 
	// in open track always populate the data, it seems opentrack always gives us a zeroed data structure to populate with pose data.
	// if we have no new data, we don't populate it and so 0 pose gets handed back which is wrong. By always running the code below, if we 
	// have no new data, we will just give it the previous pose data which is the best thing we can do really.

    if (bEnableYaw) {
        if (bInvertYaw )	data[Yaw] =  HAT.Rot[iYawAxe] *  -1.0f;
        else 	data[Yaw] = HAT.Rot[iYawAxe];
			
    } else data[Yaw] =0;

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

    new_frame=false;

	// For debug
	//data->x=dataRead.length();
	//data->y=CptError;
}

//
// Apply modification Settings 
//
void hatire::applysettings(const TrackerSettings& settings)
{
    ts.sSerialPortName = settings.SerialPortName;

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
    ts.bEnableLogging = settings.EnableLogging;

	iRollAxe= settings.RollAxe;
	iPitchAxe= settings.PitchAxe;
	iYawAxe= settings.YawAxe;
	iXAxe= settings.XAxe;
	iYAxe= settings.YAxe;
	iZAxe= settings.ZAxe;

    ts.iBaudRate=settings.pBaudRate;
    ts.iDataBits=settings.pDataBits;
    ts.iParity=settings.pParity;
    ts.iStopBits=settings.pStopBits;
    ts.iFlowControl=settings.pFlowControl;

    ts.sCmdStart= settings.CmdStart.toLatin1();
    ts.sCmdStop= settings.CmdStop.toLatin1();
    ts.sCmdInit= settings.CmdInit.toLatin1();
    ts.sCmdReset= settings.CmdReset.toLatin1();
    ts.sCmdCenter= settings.CmdCenter.toLatin1();
    ts.sCmdZero= settings.CmdZero.toLatin1();
    ts.iDelayInit=settings.DelayInit;
    ts.iDelayStart=settings.DelayStart;
    ts.iDelaySeq=settings.DelaySeq;
    ts.bBigEndian=settings.BigEndian;

    t.update_serial_settings(ts);
}

#include "ftnoir_tracker_hat_dialog.h"
OPENTRACK_DECLARE_TRACKER(hatire, TrackerControls, TrackerDll)

