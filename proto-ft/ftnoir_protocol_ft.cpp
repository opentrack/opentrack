/* Copyright (c) 2013-2015, 2017 Stanislaw Halik <sthalik@misaki.pl>
 * Copyright (c) 2015 Wim Vriend
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "compat/ndebug-guard.hpp"
#include "compat/util.hpp"

#include "ftnoir_protocol_ft.h"
#include "csv/csv.h"
#include "opentrack-library-path.h"

#include <atomic>
#include <cmath>
#include <windows.h>

freetrack::freetrack()
{
}

freetrack::~freetrack()
{
    if (shm.success())
    {
        const double tmp[6] {};
        pose(tmp);
    }

    dummyTrackIR.close();
}

static_assert(sizeof(LONG) == sizeof(std::int32_t), "");
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

never_inline void store(std::int32_t volatile& place, std::int32_t value)
{
    (void)InterlockedExchange((LONG volatile*) &place, value);
}

never_inline std::int32_t load(std::int32_t volatile& place)
{
    return InterlockedCompareExchange((volatile LONG*) &place, 0, 0);
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

    FTHeap volatile* ft = pMemData;
    FTData volatile* data = &ft->data;

    store(data->X, tx);
    store(data->Y, ty);
    store(data->Z, tz);

    store(data->Yaw, yaw);
    store(data->Pitch, pitch);
    store(data->Roll, roll);

    const std::int32_t id = load(ft->GameID);

    if (intGameID != id)
    {
        QString gamename;
        union  {
            unsigned char table[8];
            std::int32_t ints[2];
        } t;

        t.ints[0] = 0; t.ints[1] = 0;

        (void)CSV::getGameData(id, t.table, gamename);

        {
            const std::uintptr_t addr = (std::uintptr_t)(void*)&pMemData->table[0];
            const std::uintptr_t addr_ = addr & ~(sizeof(LONG)-1u);

            static_assert(sizeof(LONG) == 4, "");

            // the data `happens' to be aligned by virtue of element ordering
            // inside FTHeap. there's no deeper reason behind it.
            if (addr != addr_)
                assert(!"unaligned access");

            // no unaligned atomic access for `char'
            for (unsigned k = 0; k < 2; k++)
                store(*(std::int32_t volatile*)&pMemData->table_ints[k], t.ints[k]);
        }

        store(ft->GameID2, id);
        store((std::int32_t volatile&) data->DataID, 0);

        intGameID = id;

        QMutexLocker foo(&game_name_mutex);
        connected_game = gamename;
    }
    else
        (void)InterlockedAdd((LONG volatile*)&data->DataID, 1);
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
        return error("Can't load freetrack memory mapping");

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
        store(*(std::int32_t volatile*)&pMemData->table_ints[k], 0);

    // more games need the dummy executable than previously thought
    if (use_npclient)
        start_dummy();

    return status_ok();
}

OPENTRACK_DECLARE_PROTOCOL(freetrack, FTControls, freetrackDll)
