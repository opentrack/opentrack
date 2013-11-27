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

    QSettings settings("opentrack");
	QString currentFile = settings.value( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup( "PointTracker" );

	cam_index      = iniFile.value("CameraId",     0).toInt();
	cam_f          = iniFile.value("CameraF",      1).toFloat();
	cam_res_x      = iniFile.value("CameraResX", 640).toInt();
	cam_res_y      = iniFile.value("CameraResY", 480).toInt();
	cam_fps        = iniFile.value("CameraFPS",   30).toInt();
	cam_roll       = iniFile.value("CameraRoll",   0).toInt();
	cam_pitch      = iniFile.value("CameraPitch",  0).toInt();
	cam_yaw        = iniFile.value("CameraYaw",    0).toInt();
	threshold      = iniFile.value("PointExtractThreshold", 128).toInt();
	min_point_size = iniFile.value("PointExtractMinSize", 2).toInt();
	max_point_size = iniFile.value("PointExtractMaxSize", 50).toInt();
	M01[0]  = iniFile.value("PointModelM01x",   0).toFloat();
	M01[1]  = iniFile.value("PointModelM01y",  40).toFloat();
	M01[2]  = iniFile.value("PointModelM01z", -30).toFloat();
	M02[0]  = iniFile.value("PointModelM02x",   0).toFloat();
	M02[1]  = iniFile.value("PointModelM02y", -70).toFloat();
	M02[2]  = iniFile.value("PointModelM02z", -80).toFloat();
	t_MH[0] = iniFile.value("tMHx", 0).toFloat();
	t_MH[1] = iniFile.value("tMHy", 0).toFloat();
	t_MH[2] = iniFile.value("tMHz", 0).toFloat();
	dyn_pose_res = iniFile.value("DynamicPoseResolution", true).toBool();
	video_widget = iniFile.value("VideoWidget", true).toBool();
	sleep_time   = iniFile.value("SleepTime",   10).toInt();
	reset_time   = iniFile.value("ResetTime", 1000).toInt();

	bEnableRoll  = iniFile.value("EnableRoll",  1).toBool();
	bEnablePitch = iniFile.value("EnablePitch", 1).toBool();
	bEnableYaw   = iniFile.value("EnableYaw",   1).toBool();
	bEnableX = iniFile.value("EnableX", 1).toBool();
	bEnableY = iniFile.value("EnableY", 1).toBool();
	bEnableZ = iniFile.value("EnableZ", 1).toBool();

	iniFile.endGroup();
}

void TrackerSettings::save_ini() const
{
	qDebug("TrackerSettings::save_ini()");

    QSettings settings("opentrack");
	QString currentFile = settings.value( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "PointTracker" );

	iniFile.setValue("CameraId",       cam_index);
	iniFile.setValue("CameraF",        cam_f);
	iniFile.setValue("CameraResX",     cam_res_x);
	iniFile.setValue("CameraResY",     cam_res_y);
	iniFile.setValue("CameraFPS",      cam_fps);
	iniFile.setValue("CameraRoll",     cam_roll);
	iniFile.setValue("CameraPitch",    cam_pitch);
	iniFile.setValue("CameraYaw",      cam_yaw);
	iniFile.setValue("PointExtractThreshold", threshold);
	iniFile.setValue("PointExtractMinSize", min_point_size);
	iniFile.setValue("PointExtractMaxSize", max_point_size);
	iniFile.setValue("PointModelM01x", M01[0]);
	iniFile.setValue("PointModelM01y", M01[1]);
	iniFile.setValue("PointModelM01z", M01[2]);
	iniFile.setValue("PointModelM02x", M02[0]);
	iniFile.setValue("PointModelM02y", M02[1]);
	iniFile.setValue("PointModelM02z", M02[2]);
	iniFile.setValue("tMHx", t_MH[0]);
	iniFile.setValue("tMHy", t_MH[1]);
	iniFile.setValue("tMHz", t_MH[2]);
	iniFile.setValue("DynamicPoseResolution", dyn_pose_res);
	iniFile.setValue("VideoWidget", video_widget);
	iniFile.setValue("SleepTime", sleep_time);
	iniFile.setValue("ResetTime", reset_time);

	iniFile.setValue( "EnableRoll",  bEnableRoll );
	iniFile.setValue( "EnablePitch", bEnablePitch );
	iniFile.setValue( "EnableYaw",   bEnableYaw );
	iniFile.setValue( "EnableX", bEnableX );
	iniFile.setValue( "EnableY", bEnableY );
	iniFile.setValue( "EnableZ", bEnableZ );

	iniFile.endGroup();
}

//-----------------------------------------------------------------------------
void TrackerDialogSettings::load_ini()
{
	qDebug("TrackerDialogSettings::load_ini()");

    QSettings settings("opentrack");
	QString currentFile = settings.value( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup( "PointTrackerDialog" );

	active_model_panel = iniFile.value("ActiveModelPanel", MODEL_CLIP).toInt();
	M01x    = iniFile.value("CustomM01x",   0).toInt();
	M01y    = iniFile.value("CustomM01y",  40).toInt();
	M01z    = iniFile.value("CustomM01z", -30).toInt();
	M02x    = iniFile.value("CustomM02x",   0).toInt();
	M02y    = iniFile.value("CustomM02y", -70).toInt();
	M02z    = iniFile.value("CustomM02z", -80).toInt();
	clip_ty = iniFile.value("ClipTopHeight", 40).toInt();
	clip_tz = iniFile.value("ClipTopLength", 30).toInt();
	clip_by = iniFile.value("ClipBottomHeight", 70).toInt();
	clip_bz = iniFile.value("ClipBottomLength", 80).toInt();
	cap_x   = iniFile.value("CapHalfWidth", 40).toInt();
	cap_y   = iniFile.value("CapHeight", 60).toInt();
	cap_z   = iniFile.value("CapLength", 100).toInt();
}

void TrackerDialogSettings::save_ini() const
{
	qDebug("TrackerDialogSettings::save_ini()");

    QSettings settings("opentrack");
	QString currentFile = settings.value( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "PointTrackerDialog" );

	iniFile.setValue("ActiveModelPanel", active_model_panel);
	iniFile.setValue("CustomM01x", M01x);
	iniFile.setValue("CustomM01y", M01y);
	iniFile.setValue("CustomM01z", M01z);
	iniFile.setValue("CustomM02x", M02x);
	iniFile.setValue("CustomM02y", M02y);
	iniFile.setValue("CustomM02z", M02z);
	iniFile.setValue("ClipTopHeight", clip_ty);
	iniFile.setValue("ClipTopLength", clip_tz);
	iniFile.setValue("ClipBottomHeight", clip_by);
	iniFile.setValue("ClipBottomLength", clip_bz);
	iniFile.setValue("CapHalfWidth", cap_x);
	iniFile.setValue("CapHeight", cap_y);
	iniFile.setValue("CapLength", cap_z);
}
