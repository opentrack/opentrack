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
#include "compat/math.hpp"
#include <algorithm>

hatire::hatire()
{

    Begin.append((unsigned char) 0xAA);
    Begin.append((unsigned char) 0xAA);
    End.append((unsigned char) 0x55);
    End.append((unsigned char) 0x55);
}

hatire::~hatire() = default;

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
module_status hatire::start_tracker(QFrame*)
{
    CptError=0;
    frame_cnt=0;
    t.Log("Starting Tracker");

    serial_result ret = t.init_serial_port();

    t.start();

    switch (ret.code)
    {
    case result_ok:
        return status_ok();
    case result_error:
        return error(ret.error);
    case result_open_error:
        return error(tr("Unable to open ComPort: %1").arg(ret.error));
    default:
        return error(tr("Unknown error"));
    }
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

                data_read.remove(0, 30);
            }
            else
            {
                CptError++;
                // resync frame
                const int index = data_read.indexOf(Begin, 1);
                if (index == -1)
                    data_read.clear();
                else
                    data_read.remove(0, index);
            }
        }
    }

    if (CptError > 50)
    {
        qDebug() << "Can't find HAT frame";
        CptError=0;
    }

    for (unsigned k = 0; k < 3; k++)
        HAT.Rot[k] = std::clamp(HAT.Rot[k], -180.f, 180.f);

    const struct
    {
        double& place;
        float input;
        bool enable;
        bool sign;
    } spec[] =
    {
        { data[TX],    HAT.Trans[s.XAxis],   s.EnableX,     s.InvertX, },
        { data[TY],    HAT.Trans[s.YAxis],   s.EnableY,     s.InvertY, },
        { data[TZ],    HAT.Trans[s.ZAxis],   s.EnableZ,     s.InvertZ, },
        { data[Yaw],   HAT.Rot[s.YawAxis],   s.EnableYaw,   s.InvertYaw },
        { data[Pitch], HAT.Rot[s.PitchAxis], s.EnablePitch, s.InvertPitch },
        { data[Roll],  HAT.Rot[s.RollAxis],  s.EnableRoll,  s.InvertRoll },
    };

    for (auto& k : spec)
        k.place = (k.sign ? -1 : 1) * (k.enable ? (double)k.input : 0);
}

#include "ftnoir_tracker_hat_dialog.h"
OPENTRACK_DECLARE_TRACKER(hatire, dialog_hatire, hatire_metadata)

