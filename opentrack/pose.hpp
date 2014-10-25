#pragma once

#include <utility>
#include <algorithm>
#include "./plugin-api.hpp"

class Pose {
private:
    static constexpr double pi = 3.141592653;
    static constexpr double d2r = pi/180.0;
    static constexpr double r2d = 180./pi;

    double axes[6];
public:
    Pose() : axes {0,0,0, 0,0,0} {}

    inline operator double*() { return axes; }
    inline operator const double*() const { return axes; }

    inline double& operator()(int i) { return axes[i]; }
    inline double operator()(int i) const { return axes[i]; }
};
