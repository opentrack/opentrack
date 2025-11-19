/* Copyright (c) 2013-2015, 2017 Stanislaw Halik <sthalik@misaki.pl>
 * Copyright (c) 2015 Wim Vriend
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifdef __CLION_IDE__
#define _CRT_USE_BUILTIN_OFFSETOF
#endif

#include "compat/library-path.hpp"

#include "ftnoir_protocol_ft.h"
#include "csv/csv.h"
#include <QDir>

#include <cstddef>
#include <cmath>
#include <windows.h>

freetrack::~freetrack()
{
    //dummyTrackIR.kill(); dummyTrackIR.waitForFinished();
    if (s.ephemeral_library_location)
    {
        QSettings settings_ft("Freetrack", "FreetrackClient");
        QSettings settings_npclient("NaturalPoint", "NATURALPOINT\\NPClient Location");

        settings_ft.setValue("Path", "");
        settings_npclient.setValue("Path", "");

        if (dummyTrackIR.state() == dummyTrackIR.Running)
        {
            dummyTrackIR.kill();
            dummyTrackIR.waitForFinished(100);
        }
    }
    else
        dummyTrackIR.close();
}

static_assert(sizeof(LONG) == sizeof(std::int32_t));
static_assert(sizeof(LONG) == 4u);

tr_never_inline void store(float volatile& place, const float value)
{
    union
    {
        float f32;
        LONG i32;
    } value_ {};

    value_.f32 = value;

    static_assert(sizeof(value_) == sizeof(float));
    static_assert(offsetof(decltype(value_), f32) == offsetof(decltype(value_), i32));

    (void)InterlockedExchange((LONG volatile*)&place, value_.i32);
}

template<typename t>
static void store(t volatile& place, t value)
{
    static_assert(sizeof(t) == 4u);
    (void)InterlockedExchange((LONG volatile*) &place, (LONG)value);
}

static std::int32_t load(std::int32_t volatile& place)
{
    return InterlockedCompareExchange((volatile LONG*) &place, 0, 0);
}

void freetrack::pose(const double* headpose, const double* raw)
{
    constexpr double d2r = M_PI/180;

    const float yaw = float(-headpose[Yaw] * d2r);
    const float roll = float(headpose[Roll] * d2r);
    const float tx = float(headpose[TX] * 10);
    const float ty = float(headpose[TY] * 10);
    const float tz = float(headpose[TZ] * 10);

    // HACK: Falcon BMS makes a "bump" if pitch is over the value -sh 20170615
    const bool is_crossing_90 = std::fabs(headpose[Pitch] - 90) < .15;
    const float pitch = float(-d2r * (is_crossing_90 ? 89.86 : headpose[Pitch]));

    FTHeap* const ft = pMemData;
    FTData* const data = &ft->data;

    store(data->X, tx);
    store(data->Y, ty);
    store(data->Z, tz);

    store(data->Yaw, yaw);
    store(data->Pitch, pitch);
    store(data->Roll, roll);

    store(data->RawYaw, float(-raw[Yaw] * d2r));
    store(data->RawPitch, float(raw[Pitch] * d2r));
    store(data->RawRoll, float(raw[Roll] * d2r));
    store(data->RawX, float(raw[TX] * 10));
    store(data->RawY, float(raw[TY] * 10));
    store(data->RawZ, float(raw[TZ] * 10));

    const std::int32_t id = load(ft->GameID);

    if (intGameID != id)
    {
        QString gamename;
        union  {
            unsigned char table[8];
            std::int32_t ints[2];
        } t {};

        t.ints[0] = 0; t.ints[1] = 0;

        (void)getGameData(id, t.table, gamename);

        {
            // FTHeap pMemData happens to be aligned on a page boundary by virtue of
            // memory mapping usage (MS Windows equivalent of mmap(2)).
            static_assert((offsetof(FTHeap, table) & (sizeof(LONG)-1)) == 0);

            for (unsigned k = 0; k < 2; k++)
                store(pMemData->table_ints[k], t.ints[k]);
        }

        store(ft->GameID2, id);
        store(data->DataID, 0u);

        intGameID = id;

        if (gamename.isEmpty())
            gamename = tr("Unknown game");

        QMutexLocker foo(&game_name_mutex);
        connected_game = gamename;
    }
    else
        (void)InterlockedAdd((LONG volatile*)&data->DataID, 1);
}

QString freetrack::game_name()
{
    QMutexLocker foo(&game_name_mutex);
    return connected_game;
}

void freetrack::start_dummy() {
    static const QString program = OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH "TrackIR.exe";
    dummyTrackIR.setProgram("\"" + program + "\"");
    dummyTrackIR.start();
}

module_status freetrack::set_protocols()
{
    static const QString program_dir = OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH;

    // Registry settings (in HK_USER)
    QSettings settings_ft("Freetrack", "FreetrackClient");
    QSettings settings_npclient("NaturalPoint", "NATURALPOINT\\NPClient Location");

    QString location = *s.custom_location_pathname;

    bool use_freetrack = true, use_npclient = true;
    switch (*s.used_interface)
    {
    case settings::enable_npclient: use_freetrack = false; break;
    case settings::enable_freetrack: use_npclient = false; break;
    case settings::enable_both: use_freetrack = true; use_npclient = true; break;
    default:
        return error(tr("proto/freetrack: wrong interface selection '%1'")
               .arg(*s.used_interface));
    }

    if (!s.use_custom_location || s.custom_location_pathname->isEmpty() || !QDir{s.custom_location_pathname}.exists())
        location = program_dir;
    else
    {
        bool copy = true;

        if (use_npclient && !QFile{location + "/NPClient.dll"}.exists())
            copy &= QFile::copy(program_dir + "/NPClient.dll", location + "/NPClient.dll");
        if (use_npclient && !QFile{location + "/NPClient64.dll"}.exists())
            copy &= QFile::copy(program_dir + "/NPClient64.dll", location + "/NPClient64.dll");
        if (use_freetrack && !QFile{location + "/freetrackclient.dll"}.exists())
            copy &= QFile::copy(program_dir + "/freetrackclient.dll", location + "/freetrackclient.dll");
        if (use_freetrack && !QFile{location + "/freetrackclient64.dll"}.exists())
            copy &= QFile::copy(program_dir + "/freetrackclient64.dll", location + "/freetrackclient64.dll");

        if (!copy)
            return {tr("Can't copy library to selected custom location '%1'").arg(s.custom_location_pathname)};
    }

    location.replace('\\', '/');
    if (!location.endsWith('/'))
        location += '/';

    settings_ft.setValue("Path", use_freetrack ? location : "");
    settings_npclient.setValue("Path", use_npclient ? location : "");

    return {};
}

module_status freetrack::initialize()
{
    if (!shm.success())
        return error(tr("Can't load freetrack memory mapping"));

    if (auto ret = set_protocols(); !ret.is_ok())
        return ret;

    pMemData->data.DataID = 1;
    pMemData->data.CamWidth = 100;
    pMemData->data.CamHeight = 250;

#if 0
    store(pMemData->data.X1, float(100));
    store(pMemData->data.Y1, float(200));
    store(pMemData->data.X2, float(300));
    store(pMemData->data.Y2, float(200));
    store(pMemData->data.X3, float(300));
    store(pMemData->data.Y3, float(100));
#endif

    store(pMemData->GameID2, 0);

    for (unsigned k = 0; k < 2; k++)
        store(pMemData->table_ints[k], 0);

    // more games need the dummy executable than previously thought
    if (s.used_interface != settings::enable_freetrack)
        start_dummy();

    return status_ok();
}

OPENTRACK_DECLARE_PROTOCOL(freetrack, FTControls, freetrackDll)
