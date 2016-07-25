#include "simple-mat.hpp"
#include "opentrack-compat/pi-constant.hpp"
#include <cmath>

namespace euler {

euler_t OPENTRACK_LOGIC_EXPORT rmat_to_euler(const dmat<3, 3>& R)
{
    using std::atan2;
    using std::sqrt;

    const double cy = sqrt(R(2,2)*R(2, 2) + R(2, 1)*R(2, 1));
    const bool large_enough = cy > 1e-10;
    if (large_enough)
        return euler_t(atan2(-R(2, 1), R(2, 2)),
                       atan2(R(2, 0), cy),
                       atan2(-R(1, 0), R(0, 0)));
    else
        return euler_t(0.,
                       atan2(R(2, 0), cy),
                       atan2(R(0, 1), R(1, 1)));
}

// tait-bryan angles, not euler
rmat OPENTRACK_LOGIC_EXPORT euler_to_rmat(const euler_t& input)
{
    const double H = input(0);
    const double P = input(1);
    const double B = input(2);

    using std::cos;
    using std::sin;

    const auto c1 = cos(H);
    const auto s1 = sin(H);
    const auto c2 = cos(P);
    const auto s2 = sin(P);
    const auto c3 = cos(B);
    const auto s3 = sin(B);

    return rmat(
                // z
                c1 * c2,
                c1 * s2 * s3 - c3 * s1,
                s1 * s3 + c1 * c3 * s2,
                // y
                c2 * s1,
                c1 * c3 + s1 * s2 * s3,
                c3 * s1 * s2 - c1 * s3,
                // x
                -s2,
                c2 * s3,
                c2 * c3
                );
}

} // end ns euler
