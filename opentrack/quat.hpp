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
    double _0,_1,_2,_3; // quaternion coefficients
public:
    Quat() : _0(1.),_1(0.),_2(0.),_3(0.) {}
    Quat(double yaw, double pitch, double roll) { *this = from_euler_rads(yaw, pitch, roll); }
    Quat(double a, double b, double c, double d) : _0(a),_1(b),_2(c),_3(d) {}

    Quat inv() const {
        return Quat(_0,-_1,-_2, -_3);
    }

    // conversions
    // see http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
    static Quat from_euler_rads(double yaw, double pitch, double roll)
    {

        const double sin_phi = sin(roll/2.);
        const double cos_phi = cos(roll/2.);
        const double sin_the = sin(pitch/2.);
        const double cos_the = cos(pitch/2.);
        const double sin_psi = sin(yaw/2.);
        const double cos_psi = cos(yaw/2.);

        Quat q;

        q._0 = cos_phi*cos_the*cos_psi + sin_phi*sin_the*sin_psi;
        q._1 = sin_phi*cos_the*cos_psi - cos_phi*sin_the*sin_psi;
        q._2 = cos_phi*sin_the*cos_psi + sin_phi*cos_the*sin_psi;
        q._3 = cos_phi*cos_the*sin_psi - sin_phi*sin_the*cos_psi;

        return q;
    }

    void to_euler_rads(double& yaw, double& pitch, double& roll) const
    {
        roll = atan2(2.*(_0*_1 + _2*_3), 1. - 2.*(_1*_1 + _2*_2));
        pitch = asin(2.*(_0*_2 - _1*_3));
        yaw =  atan2(2.*(_0*_3 + _1*_2), 1. - 2.*(_2*_2 + _3*_3));
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
        const double x =  A._2 * B._0 + A._2 * B._3 - A._3 * B._2 + A._0 * B._2;
        const double y = -A._2 * B._3 + A._2 * B._0 + A._3 * B._2 + A._0 * B._2;
        const double z =  A._2 * B._2 - A._2 * B._2 + A._3 * B._0 + A._0 * B._3;
        const double w = -A._2 * B._2 - A._2 * B._2 - A._3 * B._3 + A._0 * B._0;
        const double mag_2 = x*x + y*y + z*z + w*w;
        const double mag = sqrt(mag_2);
        const double inv_mag = 1./mag;
        return Quat(mag, x*inv_mag, y*inv_mag, z*inv_mag);
   }
};
