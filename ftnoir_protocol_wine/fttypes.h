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
#ifndef INCLUDED_FTTYPES_H
#define INCLUDED_FTTYPES_H

#define WINE_SHM_NAME "facetracknoir-wine-shm"
#define WINE_MTX_NAME "facetracknoir-wine-mtx"

struct WineSHM {
    float rx, ry, rz, tx, ty, tz;
    bool stop;
};

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

#endif//INCLUDED_FTTYPES_H
