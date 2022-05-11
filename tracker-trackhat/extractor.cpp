#include "trackhat.hpp"
#include <algorithm>
#include <iterator>

void trackhat_extractor::extract_points(const pt_frame& data,
                                        pt_preview&, bool,
                                        std::vector<vec2>& points)
{
    points.clear();
    points.reserve(trackhat_camera::point_count);
    const auto& copy = data.as_const<trackhat_frame>()->points;

    for (const auto& pt : copy)
    {
        if (!pt.ok)
            continue;
        constexpr int sz = trackhat_camera::sensor_size;
        auto [ x, y ] = to_screen_pos(pt.x, pt.y, sz, sz);
        points.push_back({x, y});
    }
}
