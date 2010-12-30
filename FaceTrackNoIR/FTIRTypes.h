/********************************************************************************
* FTIRTypes			FTIRTypes contains the specific type definitions for the	*
*					Fake TrackIR protocol.					         			*
*					It was modelled after FTTypes.cpp.							*
*																				*
* Copyright (C) 2010	Wim Vriend (Developing)									*
*						Ron Hendriks (Testing and Research)						*
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
#pragma once
#ifndef INCLUDED_FTIRTYPES_H
#define INCLUDED_FTIRTYPES_H
  
#include "Windows.h" 
#include <tchar.h>
#include <stdio.h>

//
// Versioning hasn't been worked out yet...
//
// The following is the previous spec definition of versioning info -- I can probably do
// something very similar to this -- will keep you posted.
//
// request version information using 2 messages, they cannot be expected to arrive in a specific order - so always parse using the High byte
// the messages have a NPCONTROL byte in the first parameter, and the second parameter has packed bytes.
//   Message 1) (first parameter)NPCONTROL : (second parameter) (High Byte)NPVERSIONMAJOR (Low Byte) major version number data
//   Message 2) (first parameter)NPCONTROL : (second parameter) (High Byte)NPVERSIONMINOR (Low Byte) minor version number data

#define	NPQUERYVERSION	1040

// CONTROL DATA SUBFIELDS
#define	NPVERSIONMAJOR	1
#define	NPVERSIONMINOR	2

// DATA FIELDS
#define	NPControl		8	// indicates a control data field
						// the second parameter of a message bearing control data information contains a packed data format. 
						// The High byte indicates what the data is, and the Low byte contains the actual data
// roll, pitch, yaw
#define	NPRoll		1	// +/- 16383 (representing +/- 180) [data = input - 16383]
#define	NPPitch		2	// +/- 16383 (representing +/- 180) [data = input - 16383]
#define	NPYaw		4	// +/- 16383 (representing +/- 180) [data = input - 16383]

// x, y, z - remaining 6dof coordinates
#define	NPX			16	// +/- 16383 [data = input - 16383]
#define	NPY			32	// +/- 16383 [data = input - 16383]
#define	NPZ			64	// +/- 16383 [data = input - 16383]

#define	NP_AXIS_MIN				-16383
#define	NP_AXIS_MAX				16383

// raw object position from imager
#define	NPRawX		128	// 0..25600 (actual value is multiplied x 100 to pass two decimal places of precision)  [data = input / 100]
#define	NPRawY		256  // 0..25600 (actual value is multiplied x 100 to pass two decimal places of precision)  [data = input / 100]
#define	NPRawZ		512  // 0..25600 (actual value is multiplied x 100 to pass two decimal places of precision)  [data = input / 100]

// x, y, z deltas from raw imager position 
#define	NPDeltaX		1024 // +/- 2560 (actual value is multiplied x 10 to pass two decimal places of precision)  [data = (input / 10) - 256]
#define	NPDeltaY		2048 // +/- 2560 (actual value is multiplied x 10 to pass two decimal places of precision)  [data = (input / 10) - 256]
#define	NPDeltaZ		4096 // +/- 2560 (actual value is multiplied x 10 to pass two decimal places of precision)  [data = (input / 10) - 256]

// raw object position from imager
#define	NPSmoothX		8192	  // 0..32766 (actual value is multiplied x 10 to pass one decimal place of precision) [data = input / 10]
#define	NPSmoothY		16384  // 0..32766 (actual value is multiplied x 10 to pass one decimal place of precision) [data = input / 10]
#define	NPSmoothZ		32768  // 0..32766 (actual value is multiplied x 10 to pass one decimal place of precision) [data = input / 10]

// NPESULT values are returned from the Game Client API functions.
//
typedef enum tagNPResult
{
	NP_OK = 0,
    NP_ERR_DEVICE_NOT_PRESENT,
	NP_ERR_UNSUPPORTED_OS,
	NP_ERR_INVALID_ARG,
	NP_ERR_DLL_NOT_FOUND,
	NP_ERR_NO_DATA,
	NP_ERR_INTERNAL_DATA
} NPRESULT;

  static const char* FTIR_CLIENT_FILENAME = "NPClient.dll";
  static const char* FTIR_VIEWS_FILENAME = "TIRViews.dll";
  static const char* FTIR_MM_DATA = "{0F98177E-0E5C-4F86-8837-229D19B1701D}";
  static const char* FTIR_MUTEX = "FT_TIR_Mutex";
  static const char* FTIR_REGISTER_PROGRAMHANDLE = "FT_Register_Program_Handle";
  static const char* FTIR_UNREGISTER_PROGRAMHANDLE = "FT_UnRegister_Program_Handle";

typedef struct tagTrackIRData
{
	unsigned short wNPStatus;
	unsigned short wPFrameSignature;
	unsigned long  dwNPIOData;

	float fNPRoll;
	float fNPPitch;
	float fNPYaw;
	float fNPX;
	float fNPY;
	float fNPZ;
	float fNPRawX;
	float fNPRawY;
	float fNPRawZ;
	float fNPDeltaX;
	float fNPDeltaY;
	float fNPDeltaZ;
	float fNPSmoothX;
	float fNPSmoothY;
	float fNPSmoothZ;
} TRACKIRDATA, *LPTRACKIRDATA;

struct FTIRMemMap {
    // Emulators can check this
    int iRecordSize;
	TRACKIRDATA data;
    int Version;
    // Emulators should read these
    HANDLE RegisteredHandle;
    bool Transmission, Cursor;
    long int RequestFormat;
    long int ProgramProfileID;
    // Read/Write
    int LastError;
    int Param[16];
    unsigned short ClientNotify1, ClientNotify2;
    char Signature[400];
};
typedef FTIRMemMap * PFTIRMemMap;

//
// Typedef for pointer to the notify callback function that is implemented within
// the client -- this function receives head tester reports from the game client API
//
typedef NPRESULT (__stdcall *PF_NOTIFYCALLBACK)( unsigned short, unsigned short );
//
//// Typedefs for game client API functions (useful for declaring pointers to these
//// functions within the client for use during GetProcAddress() ops)
////
//typedef NPRESULT (__stdcall *PF_NP_REGISTERWINDOWHANDLE)( HWND );
//typedef NPRESULT (__stdcall *PF_NP_UNREGISTERWINDOWHANDLE)( void );
//typedef NPRESULT (__stdcall *PF_NP_REGISTERPROGRAMPROFILEID)( unsigned short );
//typedef NPRESULT (__stdcall *PF_NP_QUERYVERSION)( unsigned short* );
//typedef NPRESULT (__stdcall *PF_NP_REQUESTDATA)( unsigned short );
//typedef NPRESULT (__stdcall *PF_NP_GETDATA)( LPTRACKIRDATA );
//typedef NPRESULT (__stdcall *PF_NP_REGISTERNOTIFY)( PF_NOTIFYCALLBACK );
//typedef NPRESULT (__stdcall *PF_NP_UNREGISTERNOTIFY)( void );
//typedef NPRESULT (__stdcall *PF_NP_STARTCURSOR)( void );
//typedef NPRESULT (__stdcall *PF_NP_STOPCURSOR)( void );
//typedef NPRESULT (__stdcall *PF_NP_STARTDATATRANSMISSION)( void );
//typedef NPRESULT (__stdcall *PF_NP_STOPDATATRANSMISSION)( void );

//// Function Prototypes ///////////////////////////////////////////////
//
// Functions exported from game client API DLL ( note __stdcall calling convention
// is used for ease of interface to clients of differing implementations including
// C, C++, Pascal (Delphi) and VB. )
//
//NPRESULT __stdcall NP_RegisterWindowHandle( HWND hWnd );
//NPRESULT __stdcall NP_RegisterWindowHandle( HWND );
//NPRESULT __stdcall NP_UnregisterWindowHandle( void );
//NPRESULT __stdcall NP_RegisterProgramProfileID( unsigned short wPPID );
//NPRESULT __stdcall NP_QueryVersion( unsigned short* pwVersion );
//NPRESULT __stdcall NP_RequestData( unsigned short wDataReq );
//NPRESULT __stdcall NP_GetData( LPTRACKIRDATA pTID );
//NPRESULT __stdcall NP_RegisterNotify( PF_NOTIFYCALLBACK pfNotify );
//NPRESULT __stdcall NP_UnregisterNotify( void );
//NPRESULT __stdcall NP_StartCursor( void );
//NPRESULT __stdcall NP_StopCursor( void );
//NPRESULT __stdcall NP_StartDataTransmission( void );
//NPRESULT __stdcall NP_StopDataTransmission( void );

#endif//INCLUDED_FTIRTYPES_H
