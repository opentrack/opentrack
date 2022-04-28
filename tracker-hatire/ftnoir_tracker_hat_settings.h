/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include <QSerialPort>
#include "options/options.hpp"

using namespace options;

struct TrackerSettings : opts
{
    value<bool> EnableRoll, EnablePitch, EnableYaw, EnableX, EnableY, EnableZ;
    value<bool> InvertRoll, InvertPitch, InvertYaw, InvertX, InvertY, InvertZ;
    value<int> RollAxis, PitchAxis, YawAxis, XAxis, YAxis, ZAxis;

    value<QString> CmdStart, CmdStop, CmdInit, CmdReset, CmdCenter, CmdZero;

    value<int> DelayInit, DelayStart, DelaySeq;

    value<bool> BigEndian, EnableLogging, pDTR;

    value<QString> QSerialPortName;

    value<QSerialPort::BaudRate> pBaudRate;
    value<QSerialPort::DataBits> pDataBits;
    value<QSerialPort::Parity> pParity;
    value<QSerialPort::StopBits> pStopBits;
    value<QSerialPort::FlowControl> pFlowControl;

    TrackerSettings() :
        opts("hatire-tracker"),
        EnableRoll(b, "enable-roll", true),
        EnablePitch(b, "enable-pitch", true),
        EnableYaw(b, "enable-yaw", true),
        EnableX(b, "enable-x", false),
        EnableY(b, "enable-y", false),
        EnableZ(b, "enable-z", false),
        InvertRoll(b, "invert-roll", false),
        InvertPitch(b, "invert-pitch", false),
        InvertYaw(b, "invert-yaw", false),
        InvertX(b, "invert-x", false),
        InvertY(b, "invert-y", false),
        InvertZ(b, "invert-z", false),
        RollAxis(b, "roll-axis", 1),
        PitchAxis(b, "pitch-axis", 2),
        YawAxis(b, "yaw-axis", 0),
        XAxis(b, "x-axis", 0),
        YAxis(b, "y-axis", 2),
        ZAxis(b, "z-axis", 1),
        CmdStart(b, "start-command", ""),
        CmdStop(b, "stop-command", ""),
        CmdInit(b, "init-command", ""),
        CmdReset(b, "reset-command", ""),
        CmdCenter(b, "center-command", ""),
        CmdZero(b, "zero-command", ""),
        DelayInit(b, "init-delay", 0),
        DelayStart(b, "start-delay", 0),
        DelaySeq(b, "after-start-delay", 0),
        BigEndian(b, "is-big-endian", false),
        EnableLogging(b, "enable-logging", false),
        pDTR(b, "data-terminal-ready", false),
        QSerialPortName(b, "serial-port-name", ""),
        pBaudRate(b, "baud-rate", QSerialPort::Baud115200),
        pDataBits(b, "data-bits", QSerialPort::Data8),
        pParity(b, "parity", QSerialPort::NoParity),
        pStopBits(b, "stop-bits", QSerialPort::OneStop),
        pFlowControl(b, "flow-control", QSerialPort::HardwareControl)
    {}
};
