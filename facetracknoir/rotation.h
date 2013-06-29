/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef ROTATION_H
#define ROTATION_H
#include <cmath>
// ----------------------------------------------------------------------------
class Rotation {

public:
	Rotation() : w(1.0),x(0.0),y(0.0),z(0.0) {}
	Rotation(double yaw, double pitch, double roll) { fromEuler(yaw, pitch, roll); }
	Rotation(double x, double y, double z, double w) : x(x),y(y),z(z),w(w) {}

	Rotation inv(){	// inverse
		return Rotation(-x,-y,-z, w);
	}


	// conversions
	// see http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
	void fromEuler(double yaw, double pitch, double roll)
	{
		// Assuming the angles are in radians.
		double c1 = cos(yaw);
		double s1 = sin(yaw);
		double c2 = cos(roll);
		double s2 = sin(roll);
		double c3 = cos(pitch);
		double s3 = sin(pitch);
		w = sqrt(1.0 + c1 * c2 + c1*c3 - s1 * s2 * s3 + c2*c3) / 2.0;
		double w4 = (4.0 * w);
		x = (c2 * s3 + c1 * s3 + s1 * s2 * c3) / w4 ;
		y = (s1 * c2 + s1 * c3 + c1 * s2 * s3) / w4 ;
		z = (-s1 * s3 + c1 * s2 * c3 +s2) / w4 ;
	}

	void toEuler(double& yaw, double& pitch, double& roll)
	{
		
		yaw =  atan2(2.0*(y*w - x*z), 1.0 - 2.0*(y*y + z*z));
		roll = asin(2.0*(x*y + z*w));
		pitch = atan2(2.0*(x*w - y*z), 1.0 - 2.0*(x*x + z*z));
	}
	
/*	 const Rotation operator*(const Rotation& A, const Rotation& B)
	{
		return Rotation(A.w*B.w - A.x*B.x - A.y*B.y - A.z*B.z,	// quaternion multiplication
						A.w*B.x + A.x*B.w + A.y*B.z - A.z*B.y,
						A.w*B.y - A.x*B.z + A.y*B.w + A.z*B.x,
						A.w*B.z + A.x*B.y - A.y*B.x + A.z*B.w);
	}*/


		 const Rotation operator*(const Rotation& B)
	{
		const Rotation& A = *this;
		return Rotation(A.w*B.w - A.x*B.x - A.y*B.y - A.z*B.z,	// quaternion multiplication
						A.w*B.x + A.x*B.w + A.y*B.z - A.z*B.y,
						A.w*B.y - A.x*B.z + A.y*B.w + A.z*B.x,
						A.w*B.z + A.x*B.y - A.y*B.x + A.z*B.w);
	}

protected:
	double w,x,y,z; // quaternion coefficients
};



#endif //ROTATION_H
