#include "euler.hpp"
#include "math-imports.hpp"
#include <cmath>

// rotation order is XYZ

namespace euler {

euler_t OTR_COMPAT_EXPORT rmat_to_euler(const rmat& R)
{
    double alpha, beta, gamma;

    beta  = atan2( -R(2,0), sqrt(R(2,1)*R(2,1) + R(2,2)*R(2,2)) );
    alpha = atan2( R(1,0), R(0,0));
    gamma = atan2( R(2,1), R(2,2));

    return { alpha, -beta, gamma };
}

rmat OTR_COMPAT_EXPORT euler_to_rmat(const euler_t& e)
{
    const double X = e(2);
    const double Y = -e(1);
    const double Z = e(0);

    const double cx = cos(X);
    const double sx = sin(X);
    const double cy = cos(Y);
    const double sy = sin(Y);
    const double cz = cos(Z);
    const double sz = sin(Z);

    return {
        cy*cz,              -cy*sz,             sy,
        cz*sx*sy + cx*sz,   cx*cz - sx*sy*sz,   -cy*sx,
        -cx*cz*sy + sx*sz,  cz*sx + cx*sy*sz,   cx*cy,
    };
}

} // end ns euler
