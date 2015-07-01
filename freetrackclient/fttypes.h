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

#ifndef _MSC_VER
#   include <inttypes.h>
#else
typedef unsigned __int32 uint32_t;
typedef __int32 int32_t;
#endif

#define FREETRACK_HEAP "FT_SharedMem"
#define FREETRACK_MUTEX "FT_Mutext"

/* only 6 headpose floats and the data id are filled -sh */
typedef struct __FTData {
    volatile int DataID;
    volatile int CamWidth;
    volatile int CamHeight;
    /* virtual pose */
    volatile float Yaw;   /* positive yaw to the left */
    volatile float Pitch; /* positive pitch up */
    volatile float Roll;  /* positive roll to the left */
    volatile float X;
    volatile float Y;
    volatile float Z;
    /* raw pose with no smoothing, sensitivity, response curve etc. */
    volatile float RawYaw;
    volatile float RawPitch;
    volatile float RawRoll;
    volatile float RawX;
    volatile float RawY;
    volatile float RawZ;
    /* raw points, sorted by Y, origin top left corner */
    volatile float X1;
    volatile float Y1;
    volatile float X2;
    volatile float Y2;
    volatile float X3;
    volatile float Y3;
    volatile float X4;
    volatile float Y4;
} FTData;

/* we add some shit at the end for other legacy proto, sadly */

typedef struct __FTHeap {
    FTData data;
    volatile int32_t GameID;
    volatile unsigned char table[8];
    volatile int32_t GameID2;
} FTHeap;
