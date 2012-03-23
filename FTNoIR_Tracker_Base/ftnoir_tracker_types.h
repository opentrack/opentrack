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
		20110415 - WVR: Added overloaded operator - and -=
*/
#ifndef FTNOIR_TRACKER_TYPES_H
#define FTNOIR_TRACKER_TYPES_H

//
// x,y,z position in centimetres, yaw, pitch and roll in degrees...
//
#pragma pack(push, 2)
struct THeadPoseData {
	double x, y, z, yaw, pitch, roll;
	long frame_number;
};
#pragma pack(pop)

//
// Structure to hold all 6 DOF's
//
class T6DOF {
public:
	T6DOF( double x, double y, double z, 
		   double yaw, double pitch, double roll ) {
		position.x = x;
		position.y = y;
		position.z = z;
		position.yaw = yaw;
		position.pitch = pitch;
		position.roll = roll;
	}

	void initHeadPoseData(){
		position.x = 0.0f;
		position.y = 0.0f;
		position.z = 0.0f;
		position.yaw = 0.0f;
		position.pitch = 0.0f;
		position.roll = 0.0f;
		position.frame_number = 0;
	}
	T6DOF operator-( T6DOF &other ) {
		return T6DOF(position.x - other.position.x, position.y - other.position.y, position.z - other.position.z,
			         position.yaw - other.position.yaw, position.pitch - other.position.pitch, position.roll - other.position.roll);
	}
	T6DOF operator-=( T6DOF &other ) {
		return T6DOF(position.x - other.position.x, position.y - other.position.y, position.z - other.position.z,
			         position.yaw - other.position.yaw, position.pitch - other.position.pitch, position.roll - other.position.roll);
	}
	T6DOF operator+( T6DOF &other ) {
		return T6DOF(position.x + other.position.x, position.y + other.position.y, position.z + other.position.z,
			         position.yaw + other.position.yaw, position.pitch + other.position.pitch, position.roll + other.position.roll);
	}
	T6DOF operator+=( T6DOF &other ) {
		return T6DOF(position.x + other.position.x, position.y + other.position.y, position.z + other.position.z,
			         position.yaw + other.position.yaw, position.pitch + other.position.pitch, position.roll + other.position.roll);
	}

	THeadPoseData position;
};

#endif // FTNOIR_TRACKER_TYPES_H
