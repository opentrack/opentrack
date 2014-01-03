/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include <QtSerialPort/QSerialPort>
#include "facetracknoir/options.h"
#include <ftnoir_tracker_base/ftnoir_tracker_types.h>
using namespace options;

struct TrackerSettings
{	
    pbundle b;
    value<bool> EnableRoll,
                EnablePitch,
                EnableYaw,
                EnableX,
                EnableY,
                EnableZ,
                InvertRoll,
                InvertPitch,
                InvertYaw,
                InvertX,
                InvertY,
                InvertZ;
    value<int> RollAxe,
               PitchAxe,
               YawAxe,
               XAxe,
               YAxe,
               ZAxe;
    value<bool> BigEndian;
    value<QString> CmdStart,
                   CmdStop,
                   CmdInit,
                   CmdReset,
                   CmdCenter,
                   CmdZero;
    value<int> SerialPortName, DelayInit, DelayStart, DelaySeq;
    // unfortunately, no way to distinguish this and enum type
    // hence, string type used -sh
    value<int> pBaudRate, pDataBits, pParity, pStopBits, pFlowControl;
    TrackerSettings() :
        b(bundle("HAT")),
        EnableRoll(b, "EnableRoll", true),
        EnablePitch(b, "EnablePitch", true),
        EnableYaw(b, "EnableYaw", true),
        EnableX(b, "EnableX", true),
        EnableY(b, "EnableY", true),
        EnableZ(b, "EnableZ", true),
        InvertRoll(b, "InvertRoll", false),
        InvertPitch(b, "InvertPitch", false),
        InvertYaw(b, "InvertYaw", false),
        InvertX(b, "InvertX", false),
        InvertY(b, "InvertY", false),
        InvertZ(b, "InvertZ", false),
        RollAxe(b, "RollAe", 2),
        PitchAxe(b, "PitchAxe", 1),
        YawAxe(b, "YawAxe", 0),
        XAxe(b, "XAxe", 0),
        YAxe(b, "YAxe", 1),
        ZAxe(b, "ZAxe", 2),
        BigEndian(b, "BigEndian", false),
        CmdStart(b, "CmdStart", ""),
        CmdStop(b, "CmdStop", ""),
        CmdInit(b, "CmdInit", ""),
        CmdReset(b, "CmdReset", ""),
        CmdCenter(b, "CmdCenter", ""),
        CmdZero(b, "CmdZero", ""),
        SerialPortName(b, "PortName", 0),
        DelayInit(b, "DelayInit", 0),
        DelayStart(b, "DelayStart", 0),
        DelaySeq(b, "DelaySeq", 0),
        pBaudRate(b, "BaudRate", 0),
        pDataBits(b, "DataBits", 0),
        pParity(b, "Parity", 0),
        pStopBits(b, "StopBits", 0),
        pFlowControl(b, "FlowControl", 0)
    {
    }
};
