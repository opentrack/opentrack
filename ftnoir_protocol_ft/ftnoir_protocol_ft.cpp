/*******************************************************************************
* FaceTrackNoIR         This program is a private project of the some enthusiastic
*                                       gamers from Holland, who don't like to pay much for
*                                       head-tracking.
*
* Copyright (C) 2013    Wim Vriend (Developing)
*                                               Ron Hendriks (Researching and Testing)
*
* Homepage
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 3 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, see <http://www.gnu.org/licenses/>.
*
* FTServer              FTServer is the Class, that communicates headpose-data
*                               to games, using the FreeTrackClient.dll.
********************************************************************************/

/* Copyright (c) 2013-2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_protocol_ft.h"
#include "csv/csv.h"

FTNoIR_Protocol::FTNoIR_Protocol() :
    shm(FREETRACK_HEAP, FREETRACK_MUTEX, sizeof(FTHeap)),
    pMemData((FTHeap*) shm.ptr()),
    viewsStart(nullptr),
    viewsStop(nullptr),
    intGameID(0)
{
}

FTNoIR_Protocol::~FTNoIR_Protocol()
{
    if (viewsStop != NULL) {
        viewsStop();
        FTIRViewsLib.unload();
    }
    dummyTrackIR.terminate();
    dummyTrackIR.kill();
    dummyTrackIR.waitForFinished(50);
}

void FTNoIR_Protocol::pose(const double* headpose) {
    float yaw = -getRadsFromDegrees(headpose[Yaw]);
    float pitch = -getRadsFromDegrees(headpose[Pitch]);
    float roll = getRadsFromDegrees(headpose[Roll]);
    float tx = headpose[TX] * 10.f;
    float ty = headpose[TY] * 10.f;
    float tz = headpose[TZ] * 10.f;
    
    FTHeap* ft = pMemData;
    FTData* data = &ft->data;
    
    data->RawX = 0;
    data->RawY = 0;
    data->RawZ = 0;
    data->RawPitch = 0;
    data->RawYaw = 0;
    data->RawRoll = 0;
    
    data->X = tx;
    data->Y = ty;
    data->Z = tz;
    data->Yaw = yaw;
    data->Pitch = pitch;
    data->Roll = roll;
    
    data->X1 = data->DataID;
    data->X2 = 0;
    data->X3 = 0;
    data->X4 = 0;
    data->Y1 = 0;
    data->Y2 = 0;
    data->Y3 = 0;
    data->Y4 = 0;
    
    int32_t id = ft->GameID;
    
    if (intGameID != id)
    {
        QString gamename;
        {
            unsigned char table[8];
            for (int i = 0; i < 8; i++) table[i] = pMemData->table[i];
            CSV::getGameData(id, table, gamename);
            for (int i = 0; i < 8; i++) pMemData->table[i] = table[i];
        }
        ft->GameID2 = id;
        intGameID = id;
        QMutexLocker foo(&this->game_name_mutex);
        connected_game = gamename;
    }
    
    data->DataID += 1;
}

void FTNoIR_Protocol::start_tirviews() {
    QString aFileName = QCoreApplication::applicationDirPath() + "/TIRViews.dll";
    if ( QFile::exists( aFileName )) {
        FTIRViewsLib.setFileName(aFileName);
        FTIRViewsLib.load();

        viewsStart = (importTIRViewsStart) FTIRViewsLib.resolve("TIRViewsStart");
        if (viewsStart == NULL) {
            qDebug() << "FTServer::run() says: TIRViewsStart function not found in DLL!";
        }
        else {
            qDebug() << "FTServer::run() says: TIRViewsStart executed!";
            viewsStart();
        }

        //
        // Load the Stop function from TIRViews.dll. Call it when terminating the thread.
        //
        viewsStop = (importTIRViewsStop) FTIRViewsLib.resolve("TIRViewsStop");
        if (viewsStop == NULL) {
            qDebug() << "FTServer::run() says: TIRViewsStop function not found in DLL!";
        }
    }
}

void FTNoIR_Protocol::start_dummy() {


    QString program = QCoreApplication::applicationDirPath() + "/TrackIR.exe";
    dummyTrackIR.setProgram("\"" + program + "\"");
    dummyTrackIR.start();
}

bool FTNoIR_Protocol::correct()
{
        QSettings settings("Freetrack", "FreetrackClient");                                                     // Registry settings (in HK_USER)
        QSettings settingsTIR("NaturalPoint", "NATURALPOINT\\NPClient Location");       // Registry settings (in HK_USER)

    if (!shm.success())
        return false;

    QString aLocation =  QCoreApplication::applicationDirPath() + "/";

    switch (s.intUsedInterface) {
        case 0:                                                                 // Use both interfaces
            settings.setValue( "Path" , aLocation );
            settingsTIR.setValue( "Path" , aLocation );
            break;
        case 1:                                                                 // Use FreeTrack, disable TrackIR
            settings.setValue( "Path" , aLocation );
            settingsTIR.setValue( "Path" , "" );
            break;
        case 2:                                                                 // Use TrackIR, disable FreeTrack
            settings.setValue( "Path" , "" );
            settingsTIR.setValue( "Path" , aLocation );
            break;
        default:
            break;
    }

    if (s.useTIRViews) {
        start_tirviews();
    }

    // more games need the dummy executable than previously thought
    start_dummy();

    pMemData->data.DataID = 1;
    pMemData->data.CamWidth = 100;
    pMemData->data.CamHeight = 250;
    pMemData->GameID2 = 0;
    for (int i = 0; i < 8; i++)
        pMemData->table[i] = 0;

    return true;
}

OPENTRACK_DECLARE_PROTOCOL(FTNoIR_Protocol, FTControls, FTNoIR_ProtocolDll)
