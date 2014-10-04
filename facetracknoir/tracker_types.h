#pragma once

#include <utility>
#include <algorithm>
#include "./quat.hpp"
#include "./plugin-api.hpp"

struct T6DOF {
private:
    static constexpr double pi = 3.141592653;
    static constexpr double d2r = pi/180.0;
    static constexpr double r2d = 180./pi;

    double axes[6];
public:
    T6DOF() : axes {0,0,0, 0,0,0 } {}

    inline operator double*() { return axes; }
    inline operator const double*() const { return axes; }

    inline double& operator()(int i) { return axes[i]; }
    inline double operator()(int i) const { return axes[i]; }

    Quat quat() const
    {
        return Quat(axes[Yaw]*d2r, axes[Pitch]*d2r, axes[Roll]*d2r);
    }

    static T6DOF fromQuat(const Quat& q)
    {
        T6DOF ret;
        q.to_euler_rads(ret(Yaw), ret(Pitch), ret(Roll));
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
