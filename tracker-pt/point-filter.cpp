#include "point-filter.hpp"
#include <QDebug>

namespace pt_point_filter_impl {

void point_filter::reset()
{
    t = std::nullopt;
}

const PointOrder& point_filter::operator()(const PointOrder& input)
{
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

    const f K = progn(
        constexpr int C = 1'000'000;
        f x = (f)*s.point_filter_coefficient;
        f log10_pos = -2+(int)x, rest = 1+fmod(x, (f)1)*9;
        return C * pow((f)10, (f)-log10_pos) * rest;
    );
    f dt = (f)t->elapsed_seconds(); t->start();
    f dist[3], norm = 0;

    for (unsigned i = 0; i < 3; i++)
    {
        vec2 tmp = input[i] - state_[i];
        dist[i] = sqrt(tmp.dot(tmp));
        norm += dist[i];
    }

    if (norm < (f)1e-6)
        return state_;

    // gain
    float delta = std::clamp(norm * norm * K * dt, (f)0, (f)1);

    for (unsigned i = 0; i < 3; i++)
    {
        f x = delta * dist[i] / norm;
        state_[i] += x*(input[i] - state_[i]);
    }

    return state_;
}

point_filter::point_filter(const pt_settings& s) : s{s}
{
}

} // ns pt_point_filter_impl
