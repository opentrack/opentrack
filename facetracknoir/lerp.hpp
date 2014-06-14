#pragma once

#include "facetracknoir/timer.hpp"
#include <algorithm>
#include <cmath>

class lerp {
private:
    static const constexpr double eps = 1e-2;
    double last[2][6], cam[6], dt;
    Timer t;
public:
    lerp() :
        last { {0,0,0,0,0,0}, {0,0,0,0,0,0} }, cam {0,0,0,0,0,0}, dt(1)
    {
    }
    bool idempotentp(const double* input)
    {
        for (int i = 0; i < 6; i++)
        {
            double diff = fabs(cam[i] - input[i]);
            if (diff > eps)
                return false;
        }
        return true;
    }

    void write(const double* cam_, const double* input, double* output)
    {
        const double q = t.elapsed();
        const double d = q/dt;

        bool idem = idempotentp(cam_);

        if (!idem)
        {
            dt = q;
            t.start();
        }

        const double c = std::max(std::min(1.0, d), 0.0);

        if (!idem)
            for (int i = 0; i < 6; i++)
            {
                last[1][i] = last[0][i];
                last[0][i] = input[i];
                cam[i] = cam_[i];
            }

        for (int i = 0; i < 6; i++)
            output[i] = last[1][i] + (last[0][i] - last[1][i]) * c;
    }

    void get_state(double* state)
    {
        for (int i = 0; i < 6; i++)
            state[i] = last[0][i];
    }
};
