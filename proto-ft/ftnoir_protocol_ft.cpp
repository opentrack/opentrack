/* Copyright (c) 2013-2015 Stanislaw Halik <sthalik@misaki.pl>
 * Copyright (c) 2015 Wim Vriend
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_protocol_ft.h"
#include "csv/csv.h"
#include "opentrack-library-path.h"

#include <cmath>

check_for_first_run FTNoIR_Protocol::runonce_check = check_for_first_run();

FTNoIR_Protocol::FTNoIR_Protocol() :
    shm(FREETRACK_HEAP, FREETRACK_MUTEX, sizeof(FTHeap)),
    pMemData((FTHeap*) shm.ptr()),
    viewsStart(nullptr),
    viewsStop(nullptr),
    intGameID(0)
{
    runonce_check.set_enabled(s.close_protocols_on_exit);
    QObject::connect(&s.close_protocols_on_exit,
                     static_cast<void (base_value::*)(bool) const>(&value<bool>::valueChanged),
                     [] (bool flag) -> void { runonce_check.set_enabled(flag); });
    runonce_check.try_runonce();
}

FTNoIR_Protocol::~FTNoIR_Protocol()
{
    if (viewsStop != NULL) {
        viewsStop();
        FTIRViewsLib.unload();
    }
    dummyTrackIR.kill();
    dummyTrackIR.close();
}

void FTNoIR_Protocol::pose(const double* headpose)
{
    const float yaw = -degrees_to_rads(headpose[Yaw]);
    const float pitch = -degrees_to_rads(headpose[Pitch]);
    const float roll = degrees_to_rads(headpose[Roll]);
    const float tx = float(headpose[TX] * 10);
    const float ty = float(headpose[TY] * 10);
    const float tz = float(headpose[TZ] * 10);

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
            unsigned char table[8] = { 0,0,0,0, 0,0,0,0 };

            (void) CSV::getGameData(id, table, gamename);

            for (int i = 0; i < 8; i++)
                pMemData->table[i] = table[i];
        }
        ft->GameID2 = id;
        intGameID = id;
        QMutexLocker foo(&game_name_mutex);
        connected_game = gamename;
    }

    data->DataID += 1;
}

float FTNoIR_Protocol::degrees_to_rads(double degrees)
{
    return float(degrees*M_PI/180);
}

void FTNoIR_Protocol::start_tirviews()
{
    QString aFileName = OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH "TIRViews.dll";
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
    QString program = OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH "TrackIR.exe";
    dummyTrackIR.setProgram("\"" + program + "\"");
    dummyTrackIR.start();
}

void FTNoIR_Protocol::set_protocols(bool ft, bool npclient)
{
    const QString program_dir = OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH;

    // Registry settings (in HK_USER)
    QSettings settings_ft("Freetrack", "FreetrackClient");
    QSettings settings_npclient("NaturalPoint", "NATURALPOINT\\NPClient Location");

    if (ft)
        settings_ft.setValue("Path", program_dir);
    else
        settings_ft.setValue("Path", "");

    if (npclient)
        settings_npclient.setValue("Path", program_dir);
    else
        settings_npclient.setValue("Path", "");
}

bool FTNoIR_Protocol::correct()
{
    if (!shm.success())
        return false;

    bool use_ft = false, use_npclient = false;

    switch (s.intUsedInterface) {
    case 0:
        use_ft = true;
        use_npclient = true;
        break;
    case 1:
        use_ft = true;
        break;
    case 2:
        use_npclient = true;
        break;
    default:
        break;
    }

    set_protocols(use_ft, use_npclient);

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
