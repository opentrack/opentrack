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
#include <QCoreApplication>
#include <QSettings>
#include <QVariant>

#include "ftnoir_tracker_hat_settings.h"

void TrackerSettings::load_ini()
{
    QSettings settings("opentrack");	// Registry settings (in HK_USER)
	QString currentFile = settings.value( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup( "HAT" );

    SerialPortName=iniFile.value ( "PortName" ).toString();

	EnableRoll = iniFile.value( "EnableRoll", 1 ).toBool();
	EnablePitch = iniFile.value( "EnablePitch", 1 ).toBool();
	EnableYaw = iniFile.value( "EnableYaw", 1 ).toBool();
	EnableX = iniFile.value( "EnableX", 0 ).toBool();
	EnableY = iniFile.value( "EnableY", 0 ).toBool();
	EnableZ = iniFile.value( "EnableZ", 0 ).toBool();


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

#ifdef OPENTRACK_API
    FPSArduino=iniFile.value("FPSArduino",30).toInt();
#endif
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

    QSettings settings("opentrack");	// Registry settings (in HK_USER)
	QString currentFile = settings.value( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "HAT" );

	iniFile.setValue ( "PortName",SerialPortName );

    iniFile.setValue( "EnableRoll", EnableRoll );
	iniFile.setValue( "EnablePitch", EnablePitch );
	iniFile.setValue( "EnableYaw", EnableYaw );
	iniFile.setValue( "EnableX", EnableX );
	iniFile.setValue( "EnableY", EnableY );
	iniFile.setValue( "EnableZ", EnableZ );

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

#ifdef OPENTRACK_API
    iniFile.setValue ( "FPSArduino", FPSArduino );
#endif
    iniFile.setValue("BigEndian",BigEndian);

	iniFile.setValue("BaudRate",pBaudRate);
	iniFile.setValue("DataBits",pDataBits);
	iniFile.setValue("Parity",pParity);
	iniFile.setValue("StopBits",pStopBits);
	iniFile.setValue("FlowControl",pFlowControl);


	iniFile.endGroup();
}

