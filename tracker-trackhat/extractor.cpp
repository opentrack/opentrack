#include "trackhat.hpp"
#include <algorithm>
#include <iterator>

void trackhat_extractor::extract_points(const pt_frame& data,
                                        pt_preview&, bool,
                                        std::vector<vec2>& points)
{
    points.clear();
    points.reserve(trackhat_camera::point_count);
    trackHat_Points_t copy = data.as_const<trackhat_frame>()->points;

    std::sort(std::begin(copy.m_point), std::end(copy.m_point),
              [](trackHat_Point_t p1, trackHat_Point_t p2) {
        return p1.m_brightness > p2.m_brightness;
    });

    for (const auto& pt : copy.m_point)
    {
        if (pt.m_brightness == 0)
            continue;
        constexpr int sz = trackhat_camera::sensor_size;
        auto [ x, y ] = to_screen_pos(sz-1-pt.m_x, pt.m_y, sz, sz);
        points.push_back({x, y});
    }
}
