/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_tracker_pt.h"
#include <QCoreApplication>
#include <QSettings>

//-----------------------------------------------------------------------------
void TrackerSettings::load_ini()
{
	qDebug("TrackerSettings::load_ini()");
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup( "PointTracker" );

	cam_index      = iniFile.value("CameraId", 0).toInt();
	cam_f          = iniFile.value("CameraF", 1).toFloat();
	threshold      = iniFile.value("PointExtractThreshold", 128).toInt();
	min_point_size = iniFile.value("PointExtractMinSize", 2).toInt();
	max_point_size = iniFile.value("PointExtractMaxSize", 50).toInt();
	M01[0] = iniFile.value("PointModelM01x", 0).toFloat();
	M01[1] = iniFile.value("PointModelM01y", 40).toFloat();
	M01[2] = iniFile.value("PointModelM01z", -30).toFloat();
	M02[0] = iniFile.value("PointModelM02x", 0).toFloat();
	M02[1] = iniFile.value("PointModelM02y", -70).toFloat();
	M02[2] = iniFile.value("PointModelM02z", -80).toFloat();
	//TODO: headpos
	video_widget = iniFile.value("VideoWidget", true).toBool();
	sleep_time   = iniFile.value("SleepTime", 10).toInt();
	
	iniFile.endGroup();
}

void TrackerSettings::save_ini() const
{
	qDebug("TrackerSettings::save_ini()");
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "PointTracker" );

	iniFile.setValue("CameraId", cam_index);
	iniFile.setValue("CameraF",  cam_f);
	iniFile.setValue("PointExtractThreshold", threshold);
	iniFile.setValue("PointExtractMinSize", min_point_size);
	iniFile.setValue("PointExtractMaxSize", max_point_size);
	iniFile.setValue("PointModelM01x", M01[0]);
	iniFile.setValue("PointModelM01y", M01[1]);
	iniFile.setValue("PointModelM01z", M01[2]);
	iniFile.setValue("PointModelM02x", M02[0]);
	iniFile.setValue("PointModelM02y", M02[1]);
	iniFile.setValue("PointModelM02z", M02[2]);
	//TODO: headpos
	iniFile.setValue("VideoWidget", video_widget);
	iniFile.setValue("SleepTime", sleep_time);

	iniFile.endGroup();
}