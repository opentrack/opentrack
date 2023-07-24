#pragma once

#include <algorithm>
#include "export.hpp"
#include "compat/math.hpp"

constexpr double TO_RAD = (M_PI / 180);
constexpr double TO_DEG = (180 / M_PI);
constexpr double EPSILON = 1e-30;

struct tVector
{
    double v[3];
    tVector(double X = 0, double Y = 0, double Z = 0) {v[0]=X; v[1]=Y; v[2]=Z;}
    tVector(const double V[]) {v[0]=V[0]; v[1]=V[1]; v[2]=V[2];}

    void operator=(const tVector& other)
    {
        std::copy(other.v, other.v + 3, v);
    }
    inline const tVector operator+(const tVector& other) const
    {
        return tVector(v[0] + other.v[0], v[1] + other.v[1], v[2] + other.v[2]);
    }
    void operator+=(const tVector& other)
    {
        v[0] += other.v[0];
        v[1] += other.v[1];
        v[2] += other.v[2];
    }
    const tVector operator-(const tVector& other) const
    {
        return tVector(v[0] - other.v[0], v[1] - other.v[1], v[2] - other.v[2]);
    }
    void operator-=(const tVector& other)
    {
        v[0] -= other.v[0];
        v[1] -= other.v[1];
        v[2] -= other.v[2];
    }
    const tVector operator*(double scalar) const
    {
        return tVector(v[0] * scalar, v[1] * scalar, v[2] * scalar);
    }
    void operator*=(double scalar)
    {
        v[0] *= scalar;
        v[1] *= scalar;
        v[2] *= scalar;
    }
    const tVector operator/(double scalar) const
    {
        return *this * (1.0 / scalar);
    }
    void operator/= (double scalar)
    {
        *this *= 1.0 / scalar;
    }
};

struct tQuat
{
    double x, y, z, w;
    tQuat(double X = 0, double Y = 0, double Z = 0, double W = 1)
        {x = X; y = Y; z = Z; w = W;}
};

inline double VectorLength(const tVector& v) { return sqrt(v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2]); }
inline double VectorDistance(const tVector& v1, const tVector& v2) { return VectorLength(v2 - v1); }
inline tVector Lerp(const tVector& s, const tVector& d, const double alpha) { return s + (d - s) * alpha; }
tQuat OTR_COMPAT_EXPORT QuatFromYPR(const double YPR[]);
tQuat OTR_COMPAT_EXPORT QuatMultiply(const tQuat& qL, const tQuat& qR);
inline tQuat QuatDivide(const tQuat& qL, const tQuat& qR) { return QuatMultiply(qL, tQuat(-qR.x, -qR.y, -qR.z, qR.w)); }
inline double AngleBetween(const tQuat& S, const tQuat& D) { return TO_DEG * 2 * acos(fabs(S.x * D.x + S.y * D.y + S.z * D.z + S.w * D.w)); }
tQuat OTR_COMPAT_EXPORT Slerp(const tQuat& S, const tQuat& D, const double alpha);
void OTR_COMPAT_EXPORT QuatToYPR(const tQuat& Q, double YPR[]);
