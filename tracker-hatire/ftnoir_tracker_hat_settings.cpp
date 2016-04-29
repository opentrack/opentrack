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
#include <QCoreApplication>
#include <QSettings>
#include <QVariant>

#include "ftnoir_tracker_hat_settings.h"
#include "opentrack-compat/options.hpp"

// XXX TODO move to opentrack settings api -sh 20160410

void TrackerSettings::load_ini()
{
    QString currentFile = options::group::ini_pathname();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup( "HAT" );

    SerialPortName=iniFile.value ( "PortName" ).toString();

	EnableRoll = iniFile.value( "EnableRoll", 1 ).toBool();
	EnablePitch = iniFile.value( "EnablePitch", 1 ).toBool();
	EnableYaw = iniFile.value( "EnableYaw", 1 ).toBool();
	EnableX = iniFile.value( "EnableX", 0 ).toBool();
	EnableY = iniFile.value( "EnableY", 0 ).toBool();
	EnableZ = iniFile.value( "EnableZ", 0 ).toBool();
	EnableLogging = iniFile.value( "EnableLogging", 0).toBool();
	
	InvertRoll = iniFile.value( "InvertRoll", 1 ).toBool();
	InvertPitch = iniFile.value( "InvertPitch", 1 ).toBool();
	InvertYaw = iniFile.value( "InvertYaw", 1 ).toBool();
	InvertX = iniFile.value( "InvertX", 0 ).toBool();
	InvertY = iniFile.value( "InvertY", 0 ).toBool();
	InvertZ = iniFile.value( "InvertZ", 0 ).toBool();


	RollAxe=iniFile.value("RollAxe",1).toInt();
	PitchAxe=iniFile.value("PitchAxe",2).toInt();
	YawAxe=iniFile.value("YawAxe",0).toInt();
	XAxe=iniFile.value("XAxe",1).toInt();
	YAxe=iniFile.value("YAxe",2).toInt();
	ZAxe=iniFile.value("ZAxe",0).toInt();


    CmdStart=iniFile.value ( "CmdStart").toString();
    CmdStop=iniFile.value ( "CmdStop" ).toString();
    CmdInit=iniFile.value ( "CmdInit" ).toString();
    CmdReset=iniFile.value ( "CmdReset" ).toString();
    CmdCenter=iniFile.value ( "CmdCenter" ).toString();
    CmdZero=iniFile.value ( "CmdZero" ).toString();

	DelayInit=iniFile.value("DelayInit",0).toInt();
	DelayStart=iniFile.value("DelayStart",0).toInt();
	DelaySeq=iniFile.value("DelaySeq",0).toInt();

    FPSArduino=iniFile.value("FPSArduino",30).toInt();

	BigEndian=iniFile.value("BigEndian",0).toBool();


	pBaudRate=static_cast<QSerialPort::BaudRate>(iniFile.value("BaudRate",QSerialPort::Baud115200).toInt());
	pDataBits=static_cast<QSerialPort::DataBits>(iniFile.value("DataBits",QSerialPort::Data8).toInt());
	pParity=static_cast<QSerialPort::Parity>(iniFile.value("Parity",QSerialPort::NoParity).toInt());
	pStopBits=static_cast<QSerialPort::StopBits>(iniFile.value("StopBits",QSerialPort::OneStop).toInt());
	pFlowControl=static_cast<QSerialPort::FlowControl>(iniFile.value("FlowControl",QSerialPort::HardwareControl).toInt());

	iniFile.endGroup();
}

void TrackerSettings::save_ini() const
{

    QString currentFile = options::group::ini_pathname();

	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "HAT" );

	iniFile.setValue ( "PortName",SerialPortName );

    iniFile.setValue( "EnableRoll", EnableRoll );
	iniFile.setValue( "EnablePitch", EnablePitch );
	iniFile.setValue( "EnableYaw", EnableYaw );
	iniFile.setValue( "EnableX", EnableX );
	iniFile.setValue( "EnableY", EnableY );
	iniFile.setValue( "EnableZ", EnableZ );
	iniFile.setValue( "EnableLogging", EnableLogging );

    iniFile.setValue( "InvertRoll", InvertRoll );
	iniFile.setValue( "InvertPitch", InvertPitch );
	iniFile.setValue( "InvertYaw", InvertYaw );
	iniFile.setValue( "InvertX", InvertX );
	iniFile.setValue( "InvertY", InvertY );
	iniFile.setValue( "InvertZ", InvertZ );

	iniFile.setValue ( "RollAxe", RollAxe );
	iniFile.setValue ( "PitchAxe", PitchAxe );
	iniFile.setValue ( "YawAxe",YawAxe );
	iniFile.setValue ( "XAxe", XAxe );
	iniFile.setValue ( "YAxe", YAxe );
	iniFile.setValue ( "ZAxe", ZAxe );

	iniFile.setValue ( "CmdStart",CmdStart.toLatin1());
	iniFile.setValue ( "CmdStop",CmdStop.toLatin1());
	iniFile.setValue ( "CmdInit",CmdInit.toLatin1());
	iniFile.setValue ( "CmdReset",CmdReset.toLatin1());
	iniFile.setValue ( "CmdCenter",CmdCenter.toLatin1() );
	iniFile.setValue ( "CmdZero",CmdZero.toLatin1() );

	iniFile.setValue ( "DelayInit",DelayInit);
	iniFile.setValue ( "DelayStart",DelayStart);
	iniFile.setValue ( "DelaySeq",DelaySeq);

    iniFile.setValue ( "FPSArduino", FPSArduino );

    iniFile.setValue("BigEndian",BigEndian);

	iniFile.setValue("BaudRate",pBaudRate);
	iniFile.setValue("DataBits",pDataBits);
	iniFile.setValue("Parity",pParity);
	iniFile.setValue("StopBits",pStopBits);
	iniFile.setValue("FlowControl",pFlowControl);


	iniFile.endGroup();
}
