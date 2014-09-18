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
#define	NP_AXIS_MAX				16383

#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "ftnoir_protocol_ft/fttypes.h"

#define FT_EXPORT(t) __declspec(dllexport) t __stdcall

FT_EXPORT(bool) FTCreateMapping(void);

#if 0
static FILE *debug_stream = fopen("c:\\FreeTrackClient.log", "a");
#define dbg_report(...) if (debug_stream) { fprintf(debug_stream, __VA_ARGS__); fflush(debug_stream); }
#else
#define dbg_report(...) ((void)0)
#endif

static HANDLE hFTMemMap = 0;
static FTHeap *pMemData = 0;
static HANDLE hFTMutex = 0;
static const char* dllVersion = "1.0.0.0";
static const char* dllProvider = "FreeTrack";

static bool FTCreateMapping(void)
{
	if (pMemData != NULL) {
		return true;
	}

	hFTMemMap = CreateFileMappingA(INVALID_HANDLE_VALUE,
                                   NULL,
                                   PAGE_READWRITE,
                                   0, 
		                           sizeof(FTHeap), 
								   (LPCSTR) FT_MM_DATA);

    if (hFTMemMap == NULL)
    {
        pMemData = NULL;
        return false;
    }

    pMemData = (FTHeap*) MapViewOfFile(hFTMemMap, FILE_MAP_WRITE, 0, 0, sizeof(FTHeap));
    hFTMutex = CreateMutexA(NULL, false, FREETRACK_MUTEX);

	return true;
}

FT_EXPORT(bool) FTGetData(FTData* data)
{
  if (FTCreateMapping() == false)
      return false;

  if (hFTMutex && WaitForSingleObject(hFTMutex, 16) == WAIT_OBJECT_0) {
	if (pMemData) {
		if (pMemData->data.DataID > (1 << 29)) {
			pMemData->data.DataID = 0;
		}
		data->DataID = pMemData->data.DataID;
	}
	ReleaseMutex(hFTMutex);
  }
  return true;
}

// For some mysterious reason, the previously existing function FTReportID has been changed to FTReportName, but with an integer as argument.
// The Delphi-code from the FreeTrack repo suggest a char * as argument, so it cost me an afternoon to figure it out (and keep ArmA2 from crashing).
// Thanks guys!
//
FT_EXPORT(void) FTReportName( int name )
{
	dbg_report("FTReportName request (ID = %d).\n", name);
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

