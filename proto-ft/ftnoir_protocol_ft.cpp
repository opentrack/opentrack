/* Copyright (c) 2013-2015, 2017 Stanislaw Halik <sthalik@misaki.pl>
 * Copyright (c) 2015 Wim Vriend
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "compat/util.hpp"

#include "ftnoir_protocol_ft.h"
#include "csv/csv.h"
#include "opentrack-library-path.h"

#include <cassert>
#include <cstddef>
#include <atomic>
#include <cmath>
#include <cstdlib>

#include <windows.h>
#include <intrin.h>

static int page_size()
{
  SYSTEM_INFO system_info;
  GetSystemInfo(&system_info);
  return system_info.dwPageSize;
}

static_assert(sizeof(LONG) == 4u, "");

never_inline void store(float volatile& place, const float value)
{
    union
    {
        float f32;
        LONG i32 alignas(alignof(float));
    } value_;

    value_.f32 = value;

    static_assert(sizeof(value_) == sizeof(float), "");
    static_assert(offsetof(decltype(value_), f32) == offsetof(decltype(value_), i32), "");

    (void)InterlockedExchange((LONG volatile*)&place, value_.i32);
}

template<typename t, typename u>
force_inline
std::enable_if_t<(std::is_integral_v<t>) && sizeof(t) == 4>
store(t volatile& place, u value)
{
    (void)InterlockedExchange((LONG volatile*) &place, value);
}

force_inline std::int32_t load(std::int32_t volatile& place)
{
    return InterlockedCompareExchange((volatile LONG*) &place, 0, 0);
}

freetrack::freetrack()
{
}

freetrack::~freetrack()
{
    if (shm.success())
    {
        store(pMemData->data.DataID, 0);
        store(pMemData->GameID2, -1);
    }

    dummyTrackIR.close();
}

void freetrack::pose(const double* headpose)
{
    const float yaw = -degrees_to_rads(headpose[Yaw]);
    const float roll = degrees_to_rads(headpose[Roll]);
    const float tx = float(headpose[TX] * 10);
    const float ty = float(headpose[TY] * 10);
    const float tz = float(headpose[TZ] * 10);

    // HACK: Falcon BMS makes a "bump" if pitch is over the value -sh 20170615
    const bool is_crossing_90 = std::fabs(headpose[Pitch] - 90) < 1e-4;
    const float pitch = -degrees_to_rads(is_crossing_90 ? 89.85 : headpose[Pitch]);

    FTHeap* ft = pMemData;
    FTData* data = &ft->data;

    store(data->X, tx);
    store(data->Y, ty);
    store(data->Z, tz);

    store(data->Yaw, yaw);
    store(data->Pitch, pitch);
    store(data->Roll, roll);

    const std::int32_t id = load(ft->GameID);

    store(data->DataID, 60 * 10 + (rand() % 64));

    if (intGameID != id)
    {
        QString gamename;
        union  {
            unsigned char table[8];
            std::int32_t ints[2];
        } t;

        t.ints[0] = 0; t.ints[1] = 0;

        (void)CSV::getGameData(id, t.table, gamename);

        static_assert(sizeof(LONG) == 4, "");
        static_assert(sizeof(int) == 4, "");

        // memory mappings are page-aligned due to TLB
        if ((std::intptr_t(pMemData) & page_size() - 1) != 0)
            assert(!"proto/freetrack: memory mapping not page aligned");

        // the data happens to be aligned by virtue of element ordering
        // inside `FTHeap'. there's no deeper reason behind it.
        static_assert((offsetof(FTHeap, table) & sizeof(int) - 1) == 0, "");

        // no atomic access for `char'
        for (unsigned k = 0; k < 2; k++)
            store(pMemData->table_ints[k], t.ints[k]);

        store(ft->GameID2, id);

        intGameID = id;

        QMutexLocker foo(&game_name_mutex);
        connected_game = gamename;
    }
}

float freetrack::degrees_to_rads(double degrees)
{
    return float(degrees*M_PI/180);
}

void freetrack::start_dummy()
{
    static const QString program = OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH "TrackIR.exe";
    dummyTrackIR.setProgram("\"" + program + "\"");
    dummyTrackIR.start();
}

void freetrack::set_protocols(bool ft, bool npclient)
{
    static const QString program_dir = OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH;

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

module_status freetrack::initialize()
{
    if (!shm.success())
        return error(_("Can't load freetrack memory mapping"));

    bool use_ft = false, use_npclient = false;

    switch (s.intUsedInterface) {
    default:
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
    }

    set_protocols(use_ft, use_npclient);

    pMemData->data.CamWidth = 100;
    pMemData->data.CamHeight = 250;

#if 1
    store(pMemData->data.X1, float(100));
    store(pMemData->data.Y1, float(200));
    store(pMemData->data.X2, float(300));
    store(pMemData->data.Y2, float(200));
    store(pMemData->data.X3, float(300));
    store(pMemData->data.Y3, float(100));
#endif

    store(pMemData->GameID2, -1);
    store(pMemData->data.DataID, 0);

    for (unsigned k = 0; k < 2; k++)
        store(pMemData->table_ints[k], 0);

    // more games need the dummy executable than previously thought
    if (use_npclient)
        start_dummy();

    return status_ok();
}

OPENTRACK_DECLARE_PROTOCOL(freetrack, FTControls, freetrackDll)
