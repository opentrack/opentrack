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
class RotationType {

public:
	RotationType() : a(1.0),b(0.0),c(0.0),d(0.0) {}
	RotationType(double yaw, double pitch, double roll) { fromEuler(yaw, pitch, roll); }
	RotationType(double a, double b, double c, double d) : a(a),b(b),c(c),d(d) {}

	RotationType inv(){	// inverse
		return RotationType(a,-b,-c, -d);
	}


	// conversions
	// see http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
	void fromEuler(double yaw, double pitch, double roll)
	{

		double sin_phi = sin(roll/2.0);
		double cos_phi = cos(roll/2.0);
		double sin_the = sin(pitch/2.0);
		double cos_the = cos(pitch/2.0);
		double sin_psi = sin(yaw/2.0);
		double cos_psi = cos(yaw/2.0);

		a = cos_phi*cos_the*cos_psi + sin_phi*sin_the*sin_psi;
		b = sin_phi*cos_the*cos_psi - cos_phi*sin_the*sin_psi;
		c = cos_phi*sin_the*cos_psi + sin_phi*cos_the*sin_psi;
		d = cos_phi*cos_the*sin_psi - sin_phi*sin_the*cos_psi;
	}
    
    void toEuler(double& yaw, double& pitch, double& roll) const
    {
        roll = atan2(2.0*(a*b + c*d), 1.0 - 2.0*(b*b + c*c));
        pitch = asin(2.0*(a*c - b*d));
        yaw =  atan2(2.0*(a*d + b*c), 1.0 - 2.0*(c*c + d*d));
    }
    
    const RotationType operator*(const RotationType& B) const
    {
        const RotationType& A = *this;
        return RotationType(A.a*B.a - A.b*B.b - A.c*B.c - A.d*B.d,	// quaternion multiplication
                        A.a*B.b + A.b*B.a + A.c*B.d - A.d*B.c,
                        A.a*B.c - A.b*B.d + A.c*B.a + A.d*B.b,
                        A.a*B.d + A.b*B.c - A.c*B.b + A.d*B.a);
    }

protected:
	double a,b,c,d; // quaternion coefficients
};



#endif //ROTATION_H
