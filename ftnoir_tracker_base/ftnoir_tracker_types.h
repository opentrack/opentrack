/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2010	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage																		*
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
* This class implements a tracker-base											*
*********************************************************************************/
/*
	Modifications (last one on top):
		20120924 - C14: Moved T6DOF to separate file (not pulic interface)
		20110415 - WVR: Added overloaded operator - and -=
*/
#ifndef FTNOIR_TRACKER_TYPES_H
#define FTNOIR_TRACKER_TYPES_H

//
// x,y,z position in centimetres, yaw, pitch and roll in degrees...
//
#pragma pack(push, 2)
struct THeadPoseData {

	THeadPoseData()
		: x(0.0), y(0.0), z(0.0), yaw(0.0), pitch(0.0), roll(0.0), frame_number(0) {}

	THeadPoseData(double x, double y, double z, 
		double yaw, double pitch, double roll ) 
		: x(x), y(y), z(z), yaw(yaw), pitch(pitch), roll(roll), frame_number(0) {}

	double x, y, z, yaw, pitch, roll;
    unsigned char frame_number;
};
#pragma pack(pop)

#endif // FTNOIR_TRACKER_TYPES_H
