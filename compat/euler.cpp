#include "euler.hpp"
#include "math-imports.hpp"
#include <cmath>

namespace euler {

Pose_ rmat_to_euler(const rmat& R)
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
rmat euler_to_rmat(const Pose_& input)
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

} // end ns euler
