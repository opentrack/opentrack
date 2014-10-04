#pragma once

#include <utility>
#include <algorithm>
#include "rotation.h"
#include "plugin-api.hpp"

struct T6DOF {
private:
    static constexpr double PI = 3.14159265358979323846264;
    static constexpr double D2R = PI/180.0;
    static constexpr double R2D = 180.0/PI;

    double axes[6];
public:
    T6DOF() : axes {0,0,0, 0,0,0 } {}

    inline operator double*() { return axes; }
    inline operator const double*() const { return axes; }

    inline double& operator()(int i) { return axes[i]; }
    inline double operator()(int i) const { return axes[i]; }

    Quat quat() const
    {
        return Quat(axes[Yaw]*D2R, axes[Pitch]*D2R, axes[Roll]*D2R);
    }

    static T6DOF fromQuat(const Quat& q)
    {
        T6DOF ret;
        q.toEuler(ret(Yaw), ret(Pitch), ret(Roll));
        return ret;
    }

    T6DOF operator-(const T6DOF& B) const
    {
        const Quat q = (quat() * B.quat().inv());
        T6DOF ret = fromQuat(q);
        for (int i = TX; i < Yaw; i++)
            ret(i) = B(i);
        return ret;
    }

    T6DOF operator+(const T6DOF& B) const
    {
        const Quat q = (quat() * B.quat().inv());
        T6DOF ret = fromQuat(q);
        for (int i = TX; i < Yaw; i++)
            ret(i) = B(i);
        return ret;
    }

    T6DOF operator|(const T6DOF& replacement) const
    {
        T6DOF ret = *this;
        for (int i = 0; i < 6; i++)
        {
            static constexpr double eps = 1e-5;
            // NB replace zero-valued elements with argument's
            if (std::abs(ret(i)) < eps)
                ret(i) = replacement(i);
        }
        return ret;
    }
};
