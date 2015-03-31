/* Copyright (c) 2012-2013 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "ftnoir_filter_accela/ftnoir_filter_accela.h"
#include <algorithm>
#include <cmath>
#include <QDebug>
#include <QMutexLocker>
#include "opentrack/plugin-api.hpp"
using namespace std;

FTNoIR_Filter::FTNoIR_Filter() : first_run(true)
{
}

// vec = difference between current and last position
inline double f(double vec, double thres, double exponent)
{
	if (vec <= 0) return 0;
	
	// Want to dampen the movement depending on the amount of movement,
	// and we want to do it in a smooth way. The following function
	// looks like a good candiate: x^n/(a^n + x^n) = 1/(1 + (a/x)^n)
	// It goes into saturation for x>>a, and n controls how fast the 
	// function increases near x = zero.
	// In our terms: a == thres has the meaning of a length scale and
	// mostly determines the relaxiation speed. n == exponent has the
	// meaning of a dead zone size. The larger, the less movment for
	// small perturbations.
	const double mu = thres/vec;	
	return (1.0 + vec/thres*0.1)/(pow(mu, exponent) + 1.0);
	// Here we do things a bit differently than planned and add
	// vec/thres*0.1. This makes the function grow approximately
	// linearly with slope 1/(10 thres) for x >> a. I found this 
	// gives faster response for large movements. Otherwise it was
	// too slow. Actually, it should result in exponential decay
	// for x >> a.
}


void FTNoIR_Filter::filter(const double* input, double *output)
{
    if (first_run)
    {
        for (int i = 0; i < 6; i++)
        {
            output[i] = input[i];
            last_output[i] = input[i];
            smoothed_input[i] = input[i];
        }
        first_run = false;
        t.start();
        return;
    }

	// parameters use a logarithmic scale on the slider, so transform it back to linear scale
	// value range: 10^{-1} to 10^{3}
    const double rot_t = std::pow(10.0, s.dampening / 100.0 * 4.0 - 1.0);
    const double trans_t = std::pow(10.0, s.dampening_translation / 100.0 * 4.0 - 1.0) ;

	double exponent = s.deadzone / 100.0;
	exponent = std::pow(10.,exponent); 
	// value range: 1 to  10. exponents < 1 amplify small motions so thats no use.
	
    const double dt = t.elapsed() / 1000.;
    t.start();

	// changed all the coefficients to go in powers of 2
    double RC;
    switch (s.ewma)
    {
    default:
    case 0: // none
        RC = 0;
        break;
    case 1: // low
        RC = 0.01;
        break;
    case 2: // normal
        RC = 0.02;
        break;
    case 3: // high
        RC = 0.04;
        break;
    case 4: // extreme
        RC = 0.08;
        break;
    }
    RC *= 1.0e6; // because i changed the factor in dt from 1e-9 to 1e-3;
    
    for (int i = 0; i < 6; i++)
    {
        const double alpha = dt/(dt+RC);
        
        smoothed_input[i] = smoothed_input[i] * (1.-alpha) + input[i] * alpha;
        
        const double in = smoothed_input[i];
        
        const double vec = in - last_output[i];
        const double t = i >= 3 ? rot_t : trans_t;
        const double val = f(fabs(vec), t, exponent); // euler integration method, basically
        const double result = last_output[i] + (vec < 0 ? -1 : 1) * std::min(1.0, 30.*dt) * val;
        
        last_output[i] = output[i] = result;
    }
}

extern "C" OPENTRACK_EXPORT IFilter* GetConstructor()
{
    return new FTNoIR_Filter;
}

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
    return new FTNoIR_FilterDll;
}
