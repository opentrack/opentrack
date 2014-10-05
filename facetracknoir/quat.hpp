/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once
#include <cmath>

class Quat {
private:
    static constexpr double pi = 3.141592653;
    static constexpr double r2d = 180./pi;
    double a,b,c,d; // quaternion coefficients
public:
    Quat() : a(1.),b(0.),c(0.),d(0.) {}
    Quat(double yaw, double pitch, double roll) { from_euler_rads(yaw, pitch, roll); }
    Quat(double a, double b, double c, double d) : a(a),b(b),c(c),d(d) {}

    Quat inv(){
        return Quat(a,-b,-c, -d);
    }

    // conversions
    // see http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
    void from_euler_rads(double yaw, double pitch, double roll)
    {

        const double sin_phi = sin(roll/2.);
        const double cos_phi = cos(roll/2.);
        const double sin_the = sin(pitch/2.);
        const double cos_the = cos(pitch/2.);
        const double sin_psi = sin(yaw/2.);
        const double cos_psi = cos(yaw/2.);

        a = cos_phi*cos_the*cos_psi + sin_phi*sin_the*sin_psi;
        b = sin_phi*cos_the*cos_psi - cos_phi*sin_the*sin_psi;
        c = cos_phi*sin_the*cos_psi + sin_phi*cos_the*sin_psi;
        d = cos_phi*cos_the*sin_psi - sin_phi*sin_the*cos_psi;
    }

    void to_euler_rads(double& yaw, double& pitch, double& roll) const
    {
        roll = atan2(2.*(a*b + c*d), 1. - 2.*(b*b + c*c));
        pitch = asin(2.*(a*c - b*d));
        yaw =  atan2(2.*(a*d + b*c), 1. - 2.*(c*c + d*d));
    }

    void to_euler_degrees(double& yaw, double& pitch, double& roll) const
    {
        to_euler_rads(yaw, pitch, roll);
        yaw *= r2d;
        pitch *= r2d;
        roll *= r2d;
    }

    const Quat operator*(const Quat& B) const
    {
        const Quat& A = *this;
        return Quat(A.a*B.a - A.b*B.b - A.c*B.c - A.d*B.d,	// quaternion multiplication
                    A.a*B.b + A.b*B.a + A.c*B.d - A.d*B.c,
                    A.a*B.c - A.b*B.d + A.c*B.a + A.d*B.b,
                    A.a*B.d + A.b*B.c - A.c*B.b + A.d*B.a);
    }
};
