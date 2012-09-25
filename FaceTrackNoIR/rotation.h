/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef ROTATION_H
#define ROTATION_H

// ----------------------------------------------------------------------------
class Rotation {
	friend Rotation operator*(const Rotation& A, const Rotation& B);
public:
	Rotation() : a(1.0),b(0.0),c(0.0),d(0.0) {}
	Rotation(double yaw, double pitch, double roll) { fromEuler(yaw, pitch, roll); }
	Rotation(double a, double b, double c, double d) : a(a),b(b),c(c),d(d) {}

	Rotation inv();	// inverse

	// conversions
	void fromEuler(double yaw, double pitch, double roll);
	void toEuler(double& yaw, double& pitch, double& roll);
	
protected:
	double a,b,c,d; // quaternion coefficients
};

Rotation operator*(const Rotation& A, const Rotation& B); // composition of rotations

#endif //ROTATION_H
