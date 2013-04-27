/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2012	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage	http://facetracknoir.sourceforge.net/home/default.htm				*
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
* The FunctionConfigurator was made by Stanislaw Halik, and adapted to          *
* FaceTrackNoIR.																*
*																				*
* All credits for this nice piece of code should go to Stanislaw.				*
*																				*
* Copyright (c) 2011-2012, Stanislaw Halik <sthalik@misaki.pl>					*
* Permission to use, copy, modify, and/or distribute this						*
* software for any purpose with or without fee is hereby granted,				*
* provided that the above copyright notice and this permission					*
* notice appear in all copies.													*
********************************************************************************/
#include "qfunctionconfigurator.h"

#include <QtCore/QtPlugin>
#include "qfunctionconfiguratorplugin.h"


QFunctionConfiguratorPlugin::QFunctionConfiguratorPlugin(QObject *parent)
	: QObject(parent)
{
	initialized = false;
}

void QFunctionConfiguratorPlugin::initialize(QDesignerFormEditorInterface *)
{
	if (initialized)
		return;

	initialized = true;
}

bool QFunctionConfiguratorPlugin::isInitialized() const
{
	return initialized;
}

QWidget *QFunctionConfiguratorPlugin::createWidget(QWidget *parent)
{
	return new QFunctionConfigurator(parent);
}

QString QFunctionConfiguratorPlugin::name() const
{
	return "QFunctionConfigurator";
}

QString QFunctionConfiguratorPlugin::group() const
{
	return "My Plugins";
}

QIcon QFunctionConfiguratorPlugin::icon() const
{
	return QIcon();
}

QString QFunctionConfiguratorPlugin::toolTip() const
{
	return QString();
}

QString QFunctionConfiguratorPlugin::whatsThis() const
{
	return QString();
}

bool QFunctionConfiguratorPlugin::isContainer() const
{
	return false;
}

QString QFunctionConfiguratorPlugin::domXml() const
{
	return "<widget class=\"QFunctionConfigurator\" name=\"qFunctionA\">\n"
		" <property name=\"geometry\">\n"
		"  <rect>\n"
		"   <x>0</x>\n"
		"   <y>0</y>\n"
		"   <width>161</width>\n"
		"   <height>220</height>\n"
		"  </rect>\n"
		" </property>\n"
		" <property name=\"colorBezier\">\n"
		"  <color>\n"
		"   <red>255</red>\n"
		"   <green>170</green>\n"
		"   <blue>0</blue>\n"
		"  </color>\n"
		" </property>\n"
		" <property name=\"colorBackground\">\n"
		"  <color>\n"
		"   <red>192</red>\n"
		"   <green>192</green>\n"
		"   <blue>192</blue>\n"
		"  </color>\n"
		" </property>\n"
		" <property name=\"stringInputEGU\" stdset=\"0\">\n"
		"  <string>Input Yaw (degr.)</string>\n"
		" </property>\n"
		" <property name=\"stringOutputEGU\" stdset=\"0\">\n"
		"  <string>Output Yaw (degr.)</string>\n"
		" </property>\n"
		" <property name=\"maxInputEGU\" stdset=\"0\">\n"
		" <number>50</number>\n"
		" </property>\n"
		" <property name=\"maxOutputEGU\" stdset=\"0\">\n"
		" <number>180</number>\n"
		" </property>\n"
		" <property name=\"pixPerEGU_Input\" stdset=\"0\">\n"
		" <number>2</number>\n"
		" </property>\n"
		" <property name=\"pixPerEGU_Output\" stdset=\"0\">\n"
		" <number>1</number>\n"
		" </property>\n"
	   "</widget>\n";
}

QString QFunctionConfiguratorPlugin::includeFile() const
{
	return "qfunctionconfigurator.h";
}

Q_EXPORT_PLUGIN2(qfunctionconfigurator, QFunctionConfiguratorPlugin)
