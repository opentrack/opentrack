#include "euler.hpp"
#include "math-imports.hpp"
#include <cmath>

namespace euler {

euler_t OTR_COMPAT_EXPORT rmat_to_euler(const rmat& R)
{
    const double cy = sqrt(R(2,2)*R(2, 2) + R(2, 1)*R(2, 1));
    const bool large_enough = cy > 1e-10;
    if (large_enough)
       return {
           atan2(-R(1, 0), R(0, 0)),
           atan2(R(2, 0), cy),
           atan2(-R(2, 1), R(2, 2))
       };
    else
        return {
           atan2(R(0, 1), R(1, 1)),
           atan2(R(2, 0), cy),
           0
        };
}

// tait-bryan angles, not euler
rmat OTR_COMPAT_EXPORT euler_to_rmat(const euler_t& input)
{
    const double H = -input(0);
    const double P = -input(1);
    const double B = -input(2);

    const auto c1 = cos(H);
    const auto s1 = sin(H);
    const auto c2 = cos(P);
    const auto s2 = sin(P);
    const auto c3 = cos(B);
    const auto s3 = sin(B);

    return {
        // z
        c1*c2,
        c1*s2*s3 - c3*s1,
        s1*s3 + c1*c3*s2,
        // y
        c2*s1,
        c1*c3 + s1*s2*s3,
        c3*s1*s2 - c1*s3,
        // x
        -s2,
        c2*s3,
        c2*c3
    };
}

// https://en.wikipedia.org/wiki/Davenport_chained_rotations#Tait.E2.80.93Bryan_chained_rotations
void OTR_COMPAT_EXPORT tait_bryan_to_matrices(const euler_t& input,
                                                    rmat& r_roll,
                                                    rmat& r_pitch,
                                                    rmat& r_yaw)
{
    {
        const double phi = -input(2);
        const double sin_phi = sin(phi);
        const double cos_phi = cos(phi);

        r_roll = {
            1, 0, 0,
            0, cos_phi, -sin_phi,
            0, sin_phi, cos_phi
        };
    }

    {
        const double theta = input(1);
        const double sin_theta = sin(theta);
        const double cos_theta = cos(theta);

        r_pitch = {
            cos_theta, 0, -sin_theta,
            0, 1, 0,
            sin_theta, 0, cos_theta
        };
    }

    {
        const double psi = -input(0);
        const double sin_psi = sin(psi);
        const double cos_psi = cos(psi);

        r_yaw = {
            cos_psi, -sin_psi, 0,
            sin_psi, cos_psi, 0,
            0, 0, 1
        };
    }
}

} // end ns euler
