/***********************************************************************************
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
 * *                                                                                *
 * * The FreeTrackClient sources were translated from the original Delphi sources   *
 * * created by the FreeTrack developers.                                           *
 */

#ifndef _MSC_VER
#   warning "expect misnamed symbols"
#endif

#pragma GCC diagnostic ignored "-Wvariadic-macros"
#pragma GCC diagnostic ignored "-Wunused-parameter"

#define NP_AXIS_MAX 16383

#include <stdbool.h>
#include <string.h>
#include <windows.h>

#include "../ftnoir_protocol_ft/fttypes.h"

#define FT_EXPORT(t) __declspec(dllexport) t __stdcall

#if 0
#   include <stdio.h>
static FILE *debug_stream = fopen("c:\\FreeTrackClient.log", "a");
#   define dbg_report(...) if (debug_stream) { fprintf(debug_stream, __VA_ARGS__); fflush(debug_stream); }
#else
#define dbg_report(...) ((void)0)
#endif

static HANDLE hFTMemMap = 0;
static FTHeap* ipc_heap = 0;
static HANDLE ipc_mutex = 0;
static const char* dllVersion = "1.0.0.0";
static const char* dllProvider = "FreeTrack";

static bool impl_create_mapping(void)
{
    if (ipc_heap != NULL)
        return true;

    hFTMemMap = CreateFileMappingA(INVALID_HANDLE_VALUE,
                                   NULL,
                                   PAGE_READWRITE,
                                   0,
                                   sizeof(FTHeap),
                                   (LPCSTR) FREETRACK_HEAP);

    if (hFTMemMap == NULL)
        return (ipc_heap = NULL), false;

    ipc_heap = (FTHeap*) MapViewOfFile(hFTMemMap, FILE_MAP_WRITE, 0, 0, sizeof(FTHeap));
    ipc_mutex = CreateMutexA(NULL, false, FREETRACK_MUTEX);

    return true;
}

#pragma comment (linker, "/export:FTGetData")
FT_EXPORT(bool) FTGetData(FTData* data)
{
    if (impl_create_mapping() == false)
        return false;

    if (ipc_mutex && WaitForSingleObject(ipc_mutex, 16) == WAIT_OBJECT_0) {
        if (ipc_heap) {
            if (ipc_heap->data.DataID > (1 << 29))
                ipc_heap->data.DataID = 0;
            data->DataID = ipc_heap->data.DataID;
        }
        ReleaseMutex(ipc_mutex);
    }
    return true;
}

/*
// For some mysterious reason, the previously existing function FTReportID has been changed to FTReportName, but with an integer as argument.
// The Delphi-code from the FreeTrack repo suggest a char * as argument, so it cost me an afternoon to figure it out (and keep ArmA2 from crashing).
// Thanks guys!
*/
#pragma comment (linker, "/export:FTReportName")
FT_EXPORT(void) FTReportName( int name )
{
    dbg_report("FTReportName request (ID = %d).\n", name);
}

#pragma comment (linker, "/export:FTGetDllVersion")
FT_EXPORT(const char*) FTGetDllVersion(void)
{
    dbg_report("FTGetDllVersion request.\n");
    return dllVersion;
}

#pragma comment (linker, "/export:FTProvider")
FT_EXPORT(const char*) FTProvider(void)
{
    dbg_report("FTProvider request.\n");
    return dllProvider;
}

