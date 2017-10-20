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

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wvariadic-macros"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include <string.h>
#include <windows.h>

#include "fttypes.h"

#if !defined _WIN64
#   define FT_EXPORT(t) t __stdcall
#else
#   define FT_EXPORT(t) __declspec(dllexport) t
#endif


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

static BOOL impl_create_mapping(void)
{
    if (ipc_heap != NULL)
        return TRUE;

    hFTMemMap = CreateFileMappingA(INVALID_HANDLE_VALUE,
                                   NULL,
                                   PAGE_READWRITE,
                                   0,
                                   sizeof(FTHeap),
                                   (LPCSTR) FREETRACK_HEAP);

    if (hFTMemMap == NULL)
        return (ipc_heap = NULL), FALSE;

    ipc_heap = (FTHeap*) MapViewOfFile(hFTMemMap, FILE_MAP_WRITE, 0, 0, sizeof(FTHeap));
    ipc_mutex = CreateMutexA(NULL, FALSE, FREETRACK_MUTEX);

    return TRUE;
}

FT_EXPORT(BOOL) FTGetData(FTData* data)
{
    if (impl_create_mapping() == FALSE)
        return FALSE;

    if (ipc_mutex && WaitForSingleObject(ipc_mutex, 16) == WAIT_OBJECT_0) {
        memcpy(data, &ipc_heap->data, sizeof(FTData));
        if (ipc_heap->data.DataID > (1 << 29))
            ipc_heap->data.DataID = 0;
        ReleaseMutex(ipc_mutex);
    }
    return TRUE;
}

/*
// For some mysterious reason, the previously existing function FTReportID has been changed to FTReportName, but with an integer as argument.
// The Delphi-code from the FreeTrack repo suggest a char * as argument, so it cost me an afternoon to figure it out (and keep ArmA2 from crashing).
// Thanks guys!
*/
FT_EXPORT(void) FTReportName( int name )
{
    dbg_report("FTReportName request (ID = %d).\n", name);
}

FT_EXPORT(void) FTReportID( int name )
{
    dbg_report("FTReportID request (ID = %d).\n", name);
}

FT_EXPORT(const char*) FTGetDllVersion(void)
{
    dbg_report("FTGetDllVersion request.\n");
    return dllVersion;
}

FT_EXPORT(const char*) FTProvider(void)
{
    dbg_report("FTProvider request.\n");
    return dllProvider;
}

#if defined _MSC_VER && !defined _WIN64
#pragma comment (linker, "/export:FTReportID=_FTReportID@4")
#pragma comment (linker, "/export:FTReportName=_FTReportName@4")
#pragma comment (linker, "/export:FTGetDllVersion=_FTGetDllVersion@0")
#pragma comment (linker, "/export:FTProvider=_FTProvider@0")
#pragma comment (linker, "/export:FTGetData=_FTGetData@4")
#endif
