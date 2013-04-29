/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2012	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage:			http://facetracknoir.sourceforge.net/home/default.htm		*
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
/**
 * This file is part of libfreespace-examples.
 *
 * Copyright (c) 2009-2012, Hillcrest Laboratories, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of the Hillcrest Laboratories, Inc. nor the names
 *       of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written
 *       permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

#ifdef WIN32
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "ftnoir_tracker_hillcrest.h"
#include "facetracknoir/global-settings.h"

static struct freespace_BodyFrame cachedBodyFrame;

static void receiveMessageCallback(FreespaceDeviceId id,
                            struct freespace_message* message,
                            void* cookie,
                            int result) {
    if (result == FREESPACE_SUCCESS && message != NULL && message->messageType == FREESPACE_MESSAGE_BODYFRAME) {
        cachedBodyFrame = message->bodyFrame;
    }
}

static FreespaceDeviceId initializeFreespace() {
    struct freespace_message message;
    FreespaceDeviceId device;
    int numIds;
    int rc;

    // Initialize the freespace library
    rc = freespace_init();
    if (rc != FREESPACE_SUCCESS) {
        qDebug() << "Initialization error" << rc;
        return -1;
    }

    /** --- START EXAMPLE INITIALIZATION OF DEVICE -- **/
    rc = freespace_getDeviceList(&device, 1, &numIds);
    if (numIds == 0) {
        qDebug() << "freespaceInputThread: Didn't find any device";
        return -1;
    }

    rc = freespace_openDevice(device);
    if (rc != FREESPACE_SUCCESS) {
        qDebug() << "freespaceInputThread: Error opening device" << rc;
        return -1;
    }
    freespace_setReceiveMessageCallback(device, receiveMessageCallback, NULL);

    rc = freespace_flush(device);
    if (rc != FREESPACE_SUCCESS) {
        qDebug() << "freespaceInputThread: Error flushing device" << rc;
        return -1;
    }

    memset(&cachedBodyFrame, 0, sizeof(cachedBodyFrame));
    memset(&message, 0, sizeof(message));
    if (FREESPACE_SUCCESS == freespace_isNewDevice(device)) {
        message.messageType = FREESPACE_MESSAGE_DATAMODECONTROLV2REQUEST;
        message.dataModeControlV2Request.packetSelect = 2;
        message.dataModeControlV2Request.modeAndStatus |= 0 << 1;
    } else {
        message.messageType = FREESPACE_MESSAGE_DATAMODEREQUEST;
        message.dataModeRequest.enableBodyMotion = 1;
        message.dataModeRequest.inhibitPowerManager = 1;
    }
    rc = freespace_sendMessage(device, &message);
    if (rc != FREESPACE_SUCCESS) {
        qDebug() << "freespaceInputThread: Could not send message" << rc;
    }
    /** --- END EXAMPLE INITIALIZATION OF DEVICE -- **/

    return device;
}

static void finalizeFreespace(FreespaceDeviceId device) {
    struct freespace_message message;
    int rc;

    if (device != -1)
    {
        /** --- START EXAMPLE FINALIZATION OF DEVICE --- **/
        memset(&message, 0, sizeof(message));
        if (FREESPACE_SUCCESS == freespace_isNewDevice(device)) {
            message.messageType = FREESPACE_MESSAGE_DATAMODECONTROLV2REQUEST;
            message.dataModeControlV2Request.packetSelect = 1;
        } else {
            message.messageType = FREESPACE_MESSAGE_DATAMODEREQUEST;
            message.dataModeRequest.enableMouseMovement = 1;
        }
        rc = freespace_sendMessage(device, &message);
        if (rc != FREESPACE_SUCCESS) {
            qDebug() << "freespaceInputThread: Could not send message" << rc;
        }
        
        freespace_closeDevice(device);
    }
    /** --- END EXAMPLE FINALIZATION OF DEVICE --- **/

    freespace_exit();
}

FTNoIR_Tracker::FTNoIR_Tracker()
{
    device = initializeFreespace();
}

FTNoIR_Tracker::~FTNoIR_Tracker()
{
    finalizeFreespace(device);
}

void FTNoIR_Tracker::StartTracker(QFrame* videoFrame)
{
    loadSettings();
}

#define TO_DEG (180. / M_PI / 1e3 / 3.6)

bool FTNoIR_Tracker::GiveHeadPoseData(double *data)
{
    if (device != -1)
    {
        struct freespace_BodyFrame body;
        freespace_perform();
        body = cachedBodyFrame;
        if (bEnableYaw)
            data[RX] = body.angularVelZ * TO_DEG;
        if (bEnablePitch)
            data[RY] = body.angularVelY * TO_DEG;
        if (bEnableRoll)
            data[RZ] = body.angularVelX * TO_DEG;
    }
	return device != -1;
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Tracker::loadSettings() {

	qDebug() << "FTNoIR_Tracker::loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "hillcrest-tracker" );
	bEnableRoll = iniFile.value ( "EnableRoll", 1 ).toBool();
	bEnablePitch = iniFile.value ( "EnablePitch", 1 ).toBool();
	bEnableYaw = iniFile.value ( "EnableYaw", 1 ).toBool();
	iniFile.endGroup ();
}


////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker object.

// Export both decorated and undecorated names.
//   GetTracker     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetTracker@0  - Common name decoration for __stdcall functions in C language.
//#pragma comment(linker, "/export:GetTracker=_GetTracker@0")

extern "C" FTNOIR_TRACKER_BASE_EXPORT ITracker* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Tracker;
}
