#include "simple-mat.hpp"

namespace euler {

static constexpr double pi = 3.141592653;

enum Axis
{
    TX, TY, TZ, Yaw, Pitch, Roll
};

static constexpr double eps_ = 2e1;
static constexpr double d2r = pi / 180;
static constexpr double eps = eps_ * d2r;

euler_t euler_filter(const euler_t& rot_)
{
    using std::fabs;
    using std::copysign;

    euler_t rot(rot_);

    static constexpr double thres[] =
    {
        pi, pi/2, pi
    };

    double changed_eps[] = { 0, 0, 0 };

    for (int i = 0; i < 3; i++)
        if (fabs(rot(i)) > thres[i] - eps && fabs(rot(i)) < thres[i] + eps)
        {
            const double eps__ = copysign(eps, rot(i));
            changed_eps[i] = eps__;

            rot(i) -= eps__;
            rot = rmat_to_euler(euler_to_rmat(rot));
        }

    for (int i = 0; i < 3; i++)
    {
        rot(i) += changed_eps[i];
        if (fabs(rot(i)) > 2*pi)
            rot(i) = copysign(2*pi, rot(i));
    }
    return rot;
}

euler_t rmat_to_euler(const dmat<3, 3>& R)
{
    static constexpr double pi = 3.141592653;
    const double pitch_1 = asin(-R(0, 2));
    const double pitch_2 = pi - pitch_1;
    const double cos_p1 = cos(pitch_1), cos_p2 = cos(pitch_2);
    const double roll_1 = atan2(R(1, 2) / cos_p1, R(2, 2) / cos_p1);
    const double roll_2 = atan2(R(1, 2) / cos_p2, R(2, 2) / cos_p2);
    const double yaw_1 = atan2(R(0, 1) / cos_p1, R(0, 0) / cos_p1);
    const double yaw_2 = atan2(R(0, 1) / cos_p2, R(0, 0) / cos_p2);

    using std::fabs;

    const double err1 = fabs(pitch_1) + fabs(roll_1) + fabs(yaw_1);
    const double err2 = fabs(pitch_2) + fabs(roll_2) + fabs(yaw_2);

    if (err1 > err2)
    {
        bool fix_neg_pitch = pitch_1 < 0;
        return euler_t(yaw_2, fix_neg_pitch ? -pi - pitch_1 : pitch_2, roll_2);
    }
    else
        return euler_t(yaw_1, pitch_1, roll_1);
}

// tait-bryan angles, not euler
rmat euler_to_rmat(const double* input)
{
    static constexpr double pi = 3.141592653;
    auto H = input[0] * pi / 180;
    auto P = input[1] * pi / 180;
    auto B = input[2] * pi / 180;

    const auto c1 = cos(H);
    const auto s1 = sin(H);
    const auto c2 = cos(P);
    const auto s2 = sin(P);
    const auto c3 = cos(B);
    const auto s3 = sin(B);

    double foo[] = {
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
    };

    return dmat<3, 3>(foo);
}

} // end ns euler
