/********************************************************************************
* FTTypes			FTTypes contains th specific type definitions for the		*
*					FreeTrack protocol.						         			*
*					It was loosely translated from FTTypes.pas					*
*					which was created by the FreeTrack-team.					*
*																				*
* Copyright (C) 2010	Wim Vriend (Developing)									*
*						Ron Hendriks (Testing and Research)						*
*																				*
* Homepage				<http://www.free-track.net>								*
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
* We would like to extend our grattitude to the creators of SweetSpotter,		*
* which has become the basis of this program: "Great work guys!"				*
********************************************************************************/
#pragma once
#ifndef INCLUDED_FTTYPES_H
#define INCLUDED_FTTYPES_H
  
#include "Windows.h" 
#include <tchar.h>
#include <stdio.h>

//#include "Registry.h"

//  static const char* FT_CLIENT_LOCATION = "Software\\Freetrack\\FreetrackClient";
  static const char* FT_CLIENT_FILENAME = "FreeTrackClient.Dll";
  static const char* FT_MM_DATA = "FT_SharedMem";
  static const char* FREETRACK = "Freetrack";
  static const char* FREETRACK_MUTEX = "FT_Mutext";
  static const char* FT_PROGRAMID = "FT_ProgramID";


struct TFreeTrackData {
	int DataID;
	int CamWidth;
    int CamHeight;
    // virtual pose
    float Yaw;   // positive yaw to the left
    float Pitch; // positive pitch up
    float Roll;  // positive roll to the left
    float X;
    float Y;
    float Z;
    // raw pose with no smoothing, sensitivity, response curve etc. 
    float RawYaw;
    float RawPitch;
    float RawRoll;
    float RawX;
    float RawY;
    float RawZ;
    // raw points, sorted by Y, origin top left corner
    float X1;
    float Y1;
    float X2;
    float Y2;
    float X3;
    float Y3;
    float X4;
    float Y4;
};
typedef TFreeTrackData * PFreetrackData;

struct FTMemMap {
	TFreeTrackData data;
	HANDLE handle;
    char ProgramName[100];
};
typedef FTMemMap * PFTMemMap;

extern bool (*FTGetData) (PFreetrackData data); 

#endif//INCLUDED_FTTYPES_H
