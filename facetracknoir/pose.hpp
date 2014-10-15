#pragma once

#include <utility>
#include <algorithm>
#include "./quat.hpp"
#include "./plugin-api.hpp"

class Pose {
private:
    static constexpr double pi = 3.141592653;
    static constexpr double d2r = pi/180.0;
    static constexpr double r2d = 180./pi;

    double axes[6];
public:
    Pose() : axes {0,0,0, 0,0,0 } {}

    inline operator double*() { return axes; }
    inline operator const double*() const { return axes; }

    inline double& operator()(int i) { return axes[i]; }
    inline double operator()(int i) const { return axes[i]; }

    Quat quat() const
    {
        return Quat(axes[Yaw]*d2r, axes[Pitch]*d2r, axes[Roll]*d2r);
    }

    static Pose fromQuat(const Quat& q)
    {
        Pose ret;
        q.to_euler_degrees(ret(Yaw), ret(Pitch), ret(Roll));
        return ret;
    }

    Pose operator&(const Pose& B) const
    {
        const Quat q = quat() * B.quat().inv();
        Pose ret = fromQuat(q);
        for (int i = TX; i < TX + 3; i++)
            ret(i) = axes[i] - B.axes[i];
        return ret;
    }
};
