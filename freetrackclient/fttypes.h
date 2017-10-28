/************************************************************************************
 * * FTTypes            FTTypes contains the specific type definitions for the      *
 * *                    FreeTrack protocol.                                         *
 * *                    It was loosely translated from FTTypes.pas                  *
 * *                    which was created by the FreeTrack-team.                    *
 * *                                                                                *
 * * Wim Vriend (Developing)                                                        *
 * * Ron Hendriks (Testing and Research)                                            *
 * *                                                                                *
 * * Homepage               <http://facetracknoir.sourceforge.net/home/default.htm> *
 * *                                                                                *
 * * This program is distributed in the hope that it will be useful, but            *
 * * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY     *
 * * or FITNESS FOR A PARTICULAR PURPOSE.                                           *
 * *                                                                                *
 * * The FTTypes sources were translated from the original Delphi sources           *
 * * created by the FreeTrack developers.                                           *
 */

#pragma once

#include <inttypes.h>

#define FREETRACK_HEAP "FT_SharedMem"
#define FREETRACK_MUTEX "FT_Mutext"

/* only 6 headpose floats and the data id are filled -sh */
typedef struct FTData__ {
    uint32_t DataID;
    int32_t CamWidth;
    int32_t CamHeight;
    /* virtual pose */
    float  Yaw;   /* positive yaw to the left */
    float  Pitch; /* positive pitch up */
    float  Roll;  /* positive roll to the left */
    float  X;
    float  Y;
    float  Z;
    /* raw pose with no smoothing, sensitivity, response curve etc. */
    float  RawYaw;
    float  RawPitch;
    float  RawRoll;
    float  RawX;
    float  RawY;
    float  RawZ;
    /* raw points, sorted by Y, origin top left corner */
    float  X1;
    float  Y1;
    float  X2;
    float  Y2;
    float  X3;
    float  Y3;
    float  X4;
    float  Y4;
} volatile FTData;

/* we add some shit at the end for other legacy proto, sadly */

typedef struct FTHeap__ {
    FTData data;
    int32_t GameID;
    union
    {
        unsigned char table[8];
        int32_t table_ints[2];
    };
    int32_t GameID2;
} volatile FTHeap;
