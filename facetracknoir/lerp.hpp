#pragma once

#include "facetracknoir/timer.hpp"
#include <algorithm>

class lerp {
private:
    double last[2][6], dt;
    Timer t;
public:
    lerp() :
        last { {0,0,0,0,0,0}, {0,0,0,0,0,0} }, dt(1)
    {
    }
    bool idempotentp(const double* input)
    {
        for (int i = 0; i < 6; i++)
            if (last[0][i] != input[i])
                return false;
        return true;
    }

    void write(const double* input, double* output)
    {
        const double q = dt;
        dt = std::max(1, t.start());

        const double c = std::max(std::min(1.0, q/(double)dt), 0.0);

        for (int i = 0; i < 6; i++)
        {
            last[1][i] = last[0][i];
            last[0][i] = input[i];
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
