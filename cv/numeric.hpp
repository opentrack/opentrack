#pragma once

#include <opencv2/core.hpp>

namespace types {
    using f = double;

    static constexpr inline f eps = f(1e-8);
    static constexpr inline f pi = f(M_PI);

    template<int n> using vec = cv::Vec<f, n>;
    using vec2 = vec<2>;
    using vec3 = vec<3>;

    template<int y, int x> using mat = cv::Matx<f, y, x>;
    using mat33 = mat<3, 3>;
    using mat22 = mat<2, 2>;
}
