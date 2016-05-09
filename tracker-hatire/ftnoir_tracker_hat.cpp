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
#include <algorithm>

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
}

hatire::~hatire()
{
}

//send RESET to Arduino
void hatire::reset()
{
    t.sendcmd_str(s.CmdReset);
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

    t.start();
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

        QByteArray& data_read = t.send_data_read_nolock();

        while (data_read.length() >= 30)
        {
            //t.Log(data_read.toHex());
            // .Begin==0xAAAA .End==0x5555
            if (data_read[0] == Begin[0] && data_read[1] == Begin[1] &&
                data_read[28] == End[0] && data_read[29] == End[1])
            {
                QDataStream stream(&data_read, QIODevice::ReadOnly);

                if (s.BigEndian)
                    stream.setByteOrder(QDataStream::BigEndian);
                else
                    stream.setByteOrder(QDataStream::LittleEndian);

                stream >> ArduinoData;

                frame_cnt++;

                if (ArduinoData.Code <= 1000)
                    HAT = ArduinoData;
                else
                    emit t.serial_debug_info(data_read.mid(4,24))  ;
                data_read.remove(0, 30);
            }
            else
            {
                // resync frame
                int index =	data_read.indexOf(Begin, 1);
                if (index == -1)
                {
                    index = data_read.length();
                }

                if (data_read.length() != 0)
                {
                    emit t.serial_debug_info(data_read.mid(0,index));

                    data_read.remove(0, index);

                    CptError++;

                    qDebug() << QTime::currentTime() << "hatire resync stream" << "index" << index << "remaining" << data_read.size();
                }
            }
        }
    }

    if (CptError > 50)
    {
        emit t.serial_debug_info("Can't find HAT frame");
		CptError=0;
	}

	// Need to handle this differently in opentrack as opposed to tracknoir
    //if  (new_frame) { 
	// in open track always populate the data, it seems opentrack always gives us a zeroed data structure to populate with pose data.
	// if we have no new data, we don't populate it and so 0 pose gets handed back which is wrong. By always running the code below, if we 
	// have no new data, we will just give it the previous pose data which is the best thing we can do really.

    const struct
    {
        bool enable;
        bool sign;
        float input;
        double& place;
    } spec[] =
    {
        { s.EnableX, s.InvertX, HAT.Trans[s.XAxis], data[TX] },
        { s.EnableY, s.InvertY, HAT.Trans[s.YAxis], data[TY] },
        { s.EnableZ, s.InvertZ, HAT.Trans[s.ZAxis], data[TZ] },
        { s.EnableYaw, s.InvertYaw, HAT.Rot[s.YawAxis], data[Yaw] },
        { s.EnablePitch, s.InvertPitch, HAT.Rot[s.PitchAxis], data[Pitch] },
        { s.EnableRoll, s.InvertRoll, HAT.Rot[s.RollAxis], data[Roll] },
    };

    for (unsigned i = 0; i < sizeof(spec) / sizeof(*spec); i++)
    {
        auto& k = spec[i];
        k.place = (k.sign ? -1.f : 1.f) * (k.enable ? k.input : 0.f);
    }

	// For debug
	//data->x=dataRead.length();
	//data->y=CptError;
}

#include "ftnoir_tracker_hat_dialog.h"
OPENTRACK_DECLARE_TRACKER(hatire, TrackerControls, TrackerDll)

