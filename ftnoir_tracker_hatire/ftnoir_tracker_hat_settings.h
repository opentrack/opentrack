/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef FTNOIR_TRACKER_HAT_SETTINGS_H
#define FTNOIR_TRACKER_HAT_SETTINGS_H

#include <QtSerialPort/QSerialPort>

//-----------------------------------------------------------------------------
struct TrackerSettings
{	

	void load_ini();
	void save_ini() const;

	bool EnableRoll;
	bool EnablePitch;
	bool EnableYaw;
	bool EnableX;
	bool EnableY;
	bool EnableZ;

	bool InvertRoll;
	bool InvertPitch;
	bool InvertYaw;
	bool InvertX;
	bool InvertY;
	bool InvertZ;


	int RollAxe;
	int PitchAxe;
	int YawAxe;
	int XAxe;
	int YAxe;
	int ZAxe;

	QString  CmdStart;
	QString  CmdStop;
	QString  CmdInit;
	QString  CmdReset;
	QString  CmdCenter;
	QString  CmdZero;

	int DelayInit;
	int DelayStart;
	int DelaySeq;

	bool BigEndian;

	QString SerialPortName;
	QSerialPort::BaudRate pBaudRate;
	QSerialPort::DataBits pDataBits;
	QSerialPort::Parity pParity;
	QSerialPort::StopBits pStopBits;
	QSerialPort::FlowControl pFlowControl;

#ifdef OPENTRACK_API
    int FPSArduino;
#endif


};


#endif //FTNOIR_TRACKER_HAT_SETTINGS_H
