/********************************************************************************
* FreeTrackClientDll Implements the FreeTrack 2.0 interface for FT-enabled		*
*					games.														*
*					It uses the FreeTrack protocol (memory mapping) to			*
*					receive data from FaceTrackNoIR (or FreeTrack, or ...).		*
*																				*
* Copyright (C) 2013	Wim Vriend (Developing)									*
*						Ron Hendriks (Testing and Research)						*
*																				*
* Homepage				<http://facetracknoir.sourceforge.net/home/default.htm>	*
*																				*
* This program is free software; you can redistribute it and/or modify it		*
* under the terms of the GNU General Public License as published by the			*
* Free Software Foundation; either version 3 of the License, or (at your		*
* option) any later version.													*
*																				*
* This program is distributed in the hope that it will be useful, but			*
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY	*
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for	*
* more details.																	*
*																				*
* You should have received a copy of the GNU General Public License along		*
* with this program; if not, see <http://www.gnu.org/licenses/>.				*
*																				*
********************************************************************************/
/*
	Modifications (last one on top):

	20130208 - WVR: The old DLL from FreeTrack seems to crash ArmA2. And we need 64-bit support.
	
*/

#ifdef WIN64
#pragma comment(linker, "/export:FTGetData")
#pragma comment(linker, "/export:FTReportName")
#pragma comment(linker, "/export:FTGetDllVersion")
#pragma comment(linker, "/export:FTProvider")
#endif

#ifdef WIN64
#  define FT_DECLSPEC __declspec(dllexport)
#else
#  define FT_DECLSPEC 
#endif

#define FT_EXPORT(t) extern "C" t FT_DECLSPEC __stdcall
#define	NP_AXIS_MAX				16383

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <tchar.h>

#include "..\..\FTNOIR_PROTOCOL_FT\FTTypes.h"

//
// Functions to create/open the file-mapping
// and to destroy it again.
//
bool FTCreateMapping();
void FTDestroyMapping();
float scale2AnalogLimits( float x, float min_x, float max_x );
float getDegreesFromRads ( float rads );

#if 1
static FILE *debug_stream = fopen("c:\\FreeTrackClient.log", "a");
#define dbg_report(...) if (debug_stream) { fprintf(debug_stream, __VA_ARGS__); fflush(debug_stream); }
#else
#define dbg_report(...)
#endif

//
// Handles to 'handle' the memory mapping
//
static HANDLE hFTMemMap = 0;
static FTMemMap *pMemData = 0;
static HANDLE hFTMutex = 0;
static char* dllVersion = "1.0.0.0";
static char* dllProvider = "FreeTrack";

static unsigned short gameid = 0;

//
// DllMain gets called, when the DLL is (un)loaded or a process attaches.
//
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
#ifdef WIN64
		    dbg_report("\n= WIN64 =========================================================================================\n");
#else
		    dbg_report("\n= WIN32 =========================================================================================\n");
#endif
			dbg_report("DllMain: (0x%p, %d, %p)\n", hinstDLL, fdwReason, lpvReserved);
            dbg_report("DllMain: Attach request\n");
            DisableThreadLibraryCalls(hinstDLL);
            break;

		case DLL_PROCESS_DETACH:
			dbg_report("DllMain: Detach\n");
			dbg_report("DllMain: (0x%p, %d, %p)\n", hinstDLL, fdwReason, lpvReserved);
		    dbg_report("==========================================================================================\n");
            break;
    }
    return TRUE;
}

/******************************************************************
 *		FTGetData (FreeTrackClient.1)
 */
#pragma comment(linker, "/export:FTGetData@4=FTGetData")
FT_EXPORT(bool) FTGetData(PFreetrackData data)
{
  static int frame = 0;
  static int prevDataID = 0;
  static int dlyTrackingOff = 0;
  static int tracking = 0;

//  dbg_report("NP_GetData called.");
  if (FTCreateMapping() == false) return false;

  if (hFTMutex && WaitForSingleObject(hFTMutex, 5) == WAIT_OBJECT_0) {
	if (pMemData) {

		//
		// When FaceTrackNoIR does not update frames (any more), don't update the data.
		//
		if (prevDataID != pMemData->data.DataID) {
			memcpy(data, &pMemData->data, sizeof(TFreeTrackData));
			dlyTrackingOff = 0;
		}
		else {
			dlyTrackingOff++;
			if (dlyTrackingOff > 20) {
				dlyTrackingOff = 100;
				tracking = false;
			}
		}
		prevDataID = pMemData->data.DataID;
		
		//
		// Limit the range of DataID
		//
		if (pMemData->data.DataID > 1000) {
			pMemData->data.DataID = 0;
		}
		data->DataID = pMemData->data.DataID;

		//
		// Send the ID to FaceTrackNoIR, so it can display the game-name.
		// This could be a FreeTrack-specific ID
		//
		sprintf_s(pMemData->GameID, 19, "%d", gameid );

	}
	ReleaseMutex(hFTMutex);
  }
  return true;
}

/******************************************************************
 *		FTReportName (FreeTrackClient.2)
 */
#pragma comment(linker, "/export:FTReportName@4=FTReportName")
//
// For some mysterious reason, the previously existing function FTReportID has been changed to FTReportName, but with an integer as argument.
// The Delphi-code from the FreeTrack repo suggest a char * as argument, so it cost me an afternoon to figure it out (and keep ArmA2 from crashing).
// Thanks guys!
//
FT_EXPORT(void) FTReportName( int name )
{
	dbg_report("FTReportName request (ID = %d).\n", name);
	gameid = name;												// They might have really passed the name here... but they didn't!
	return;
}

/******************************************************************
 *		FTGetDllVersion (FreeTrackClient.3)
 */
#pragma comment(linker, "/export:FTGetDllVersion@0=FTGetDllVersion")
extern "C" __declspec( dllexport ) char* FTGetDllVersion(void)
{
    dbg_report("FTGetDllVersion request.\n");

	return dllVersion;
}

/******************************************************************
 *		FTProvider (FreeTrackClient.4)
 */
#pragma comment(linker, "/export:FTProvider@0=FTProvider")
extern "C" __declspec( dllexport ) char* FTProvider(void)
{
    dbg_report("FTProvider request.\n");

	return dllProvider;
}

//
// Create a memory-mapping to the Freetrack data.
// It contains the tracking data, a handle to the main-window and the program-name of the Game!
//
//
bool FTCreateMapping()
{
	bool bMappingExists = false;
	PDWORD_PTR MsgResult = 0;

	//
	// Memory-mapping already exists!
	//
	if ( pMemData != NULL ) {
		return true;
	}

    dbg_report("FTCreateMapping request (pMemData == NULL).\n");

	//
	// A FileMapping is used to create 'shared memory' between the FTClient and the FTServer.
	//
	// Try to create a FileMapping to the Shared Memory. This is done to check if it's already there (what
	// may mean the face-tracker program is already running).
	//
	// If one already exists: close it and open the file-mapping to the existing one.
	//
	hFTMemMap = CreateFileMappingA( INVALID_HANDLE_VALUE , 00 , PAGE_READWRITE , 0 , 
		                           sizeof( FTMemMap ), 
								   (LPCSTR) FT_MM_DATA );

	if ( ( hFTMemMap != 0 ) && ( GetLastError() == ERROR_ALREADY_EXISTS ) ) {
		dbg_report("FTCreateMapping: Mapping already exists.\n");
		bMappingExists = true;				// So the server was (probably) already started!
		CloseHandle( hFTMemMap );
		hFTMemMap = 0;
	}

	//
	// Create a new FileMapping, Read/Write access
	//
	hFTMemMap = OpenFileMappingA( FILE_MAP_ALL_ACCESS , false , (LPCSTR) FT_MM_DATA );
	if ( ( hFTMemMap != 0 ) ) {
		dbg_report("FTCreateMapping: Mapping opened.\n");
		pMemData = (FTMemMap *) MapViewOfFile(hFTMemMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof( FTMemMap ) );
	    hFTMutex = CreateMutexA(NULL, false, FREETRACK_MUTEX);
	}
	else {
		return false;
	}
	return true;
}

//
// Destory the FileMapping to the shared memory
//
void FTDestroyMapping()
{
	if ( pMemData != NULL ) {
		UnmapViewOfFile ( pMemData );
	}
	
	CloseHandle( hFTMutex );
	CloseHandle( hFTMemMap );
	pMemData = 0;
	hFTMemMap = 0;
}

//
// 4 convenience
//
float getDegreesFromRads ( float rads ) { 
	return (rads * 57.295781f); 
}

//
// Scale the measured value to the TIR values
//
float scale2AnalogLimits( float x, float min_x, float max_x ) {
double y;
double local_x;
	
	local_x = x;
	if (local_x > max_x) {
		local_x = max_x;
	}
	if (local_x < min_x) {
		local_x = min_x;
	}
	y = ( NP_AXIS_MAX * local_x ) / max_x;

	return (float) y;
}
