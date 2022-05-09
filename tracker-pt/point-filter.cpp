#include "point-filter.hpp"
#include <algorithm>
#include <cmath>
#include <QDebug>

namespace pt_point_filter_impl {

void point_filter::reset()
{
    t = std::nullopt;
}

const PointOrder& point_filter::operator()(const PointOrder& input, f deadzone_amount)
{
    using std::fmod;
    using std::sqrt;
    using std::pow;
    using std::clamp;

    if (!s.enable_point_filter)
    {
        t = std::nullopt;
        state_ = input;
        return state_;
    }

    if (!t)
    {
        t.emplace();
        state_ = input;
        return state_;
    }

    constexpr auto E = (f)1.75;
    const f limit = (f)*s.point_filter_limit;
    const f C = progn(
        constexpr int A = 1'000'000;
        double K = *s.point_filter_coefficient;
        f log10_pos = -2 + (int)K, rest = (f)(.999-fmod(K, 1.)*.9);
        return A * pow((f)10, (f)-log10_pos) * rest;
    );

    f dist = 0, dz = deadzone_amount * (f)s.point_filter_deadzone / 800; // sqrt(640^2 + 480^2)

    for (unsigned i = 0; i < 3; i++)
    {
        vec2 tmp = input[i] - state_[i];
        f x = sqrt(tmp.dot(tmp));
        x = std::max((f)0, x - dz);
        dist = std::max(dist, x);
    }

    if (dist < (f)1e-6)
        return state_;

    f dt = (f)t->elapsed_seconds(); t->start();
    f delta = pow(dist, E) * C * dt; // gain

    //qDebug() << "gain" << std::min((f)1, delta);

    for (unsigned i = 0; i < 3; i++)
    {
        f x = clamp(delta, (f)0, limit);
        state_[i] += x*(input[i] - state_[i]);
    }

    return state_;
}

point_filter::point_filter(const pt_settings& s) : s{s} {}

} // ns pt_point_filter_impl
