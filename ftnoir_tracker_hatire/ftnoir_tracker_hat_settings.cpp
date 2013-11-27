#include <QCoreApplication>
#include <QSettings>

#include "ftnoir_tracker_hat_settings.h"

void TrackerSettings::load_ini()
{
	qDebug("TrackerSettings::load_ini()");
    QSettings settings("opentrack");
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


    RollAxis=iniFile.value("RollAxis",1).toInt();
    PitchAxis=iniFile.value("PitchAxis",2).toInt();
    YawAxis=iniFile.value("YawAxis",0).toInt();
    XAxis=iniFile.value("XAxis",1).toInt();
    YAxis=iniFile.value("YAxis",2).toInt();
    ZAxis=iniFile.value("ZAxis",0).toInt();

	iniFile.endGroup();
}


void TrackerSettings::save_ini() const
{
	qDebug("TrackerSettings::save_ini()");

    QSettings settings("opentrack");
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

    iniFile.setValue ( "RollAxis", RollAxis );
    iniFile.setValue ( "PitchAxis", PitchAxis );
    iniFile.setValue ( "YawAxis",YawAxis );
    iniFile.setValue ( "XAxis", XAxis );
    iniFile.setValue ( "YAxis", YAxis );
    iniFile.setValue ( "ZAxis", ZAxis );

	iniFile.endGroup();
}
