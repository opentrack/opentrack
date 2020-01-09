/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "edtracker.h"
#include "api/plugin-api.hpp"
#include "compat/math.hpp"
#include <QMutexLocker>

edtracker::edtracker()
{
    if (s.guid->isEmpty())
    {
        std::vector<win32_joy_ctx::joy_info> info = joy_ctx.get_joy_info();
        if (!info.empty())
        {
            s.guid = info[0].guid;
            s.b->save();
        }
    }
    if (s.com_port_name->isEmpty())
    {
        for (const QSerialPortInfo& port_info : QSerialPortInfo::availablePorts()) {
            s.com_port_name = port_info.portName();
            s.b->save();
            break;
        }
    }
}

edtracker::~edtracker() = default;

void edtracker::data(double *data)
{
    int map[6] = {
        s.joy_1 - 1,
        s.joy_2 - 1,
        s.joy_3 - 1,
        s.joy_4 - 1,
        s.joy_5 - 1,
        s.joy_6 - 1,
    };

    bool map_abs[6] = {
        s.joy_1_abs,
        s.joy_2_abs,
        s.joy_3_abs,
        s.joy_4_abs,
        s.joy_5_abs,
        s.joy_6_abs,
    };

    const double limits[] = {
        100,
        100,
        100,
        180,
        180,
        180
    };

    const QString guid = s.guid;
    int axes[8];
    const bool ret = joy_ctx.poll_axis(guid, axes);

    if (ret)
    {
        for (int i = 0; i < 6; i++)
        {
            int k = map[i] - 1;
            if (k < 0 || k >= 8)
                data[i] = 0;
            else
                data[i] = clamp(axes[k] * limits[i] / AXIS_MAX,
                                -limits[i], limits[i]);
            if (map_abs[i] == true)
                data[i] = abs(data[i]);
        }
    }
}

void edtracker::reset()
{
    const QString com_port_name = s.com_port_name;
    QSerialPort com_port;
    com_port.setPortName(com_port_name);
    if (com_port.open(QIODevice::ReadWrite)) {
        com_port.setBaudRate(QSerialPort::Baud9600);
        com_port.setDataBits(QSerialPort::Data8);
        com_port.setParity(QSerialPort::NoParity);
        com_port.setStopBits(QSerialPort::OneStop);
        com_port.setFlowControl(QSerialPort::NoFlowControl);
        com_port.clear(QSerialPort::AllDirections);
        QByteArray cmd("R");
        com_port.write(cmd);
        com_port.waitForBytesWritten(1000);
        com_port.close();
    }
}

void edtracker::calibrate()
{
    const QString com_port_name = s.com_port_name;
    QSerialPort com_port;
    com_port.setPortName(com_port_name);
    if (com_port.open(QIODevice::ReadWrite)) {
        com_port.setBaudRate(QSerialPort::Baud9600);
        com_port.setDataBits(QSerialPort::Data8);
        com_port.setParity(QSerialPort::NoParity);
        com_port.setStopBits(QSerialPort::OneStop);
        com_port.setFlowControl(QSerialPort::NoFlowControl);
        com_port.clear(QSerialPort::AllDirections);
        QByteArray cmd("r");
        com_port.write(cmd);
        com_port.waitForBytesWritten(1000);
        com_port.close();
    }
}

OPENTRACK_DECLARE_TRACKER(edtracker, dialog_edtracker, edtrackerDll)
