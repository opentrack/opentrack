#pragma once

#include <opencv2/core.hpp>
#include <limits>

namespace types {
    using f = double;

    struct constants final
    {
        constants() = delete;
        static constexpr f eps = f(1e-8);
    };

    template<int n> using vec = cv::Vec<f, n>;
    using vec2 = vec<2>;
    using vec3 = vec<3>;

    template<int y, int x> using mat = cv::Matx<f, y, x>;
    using mat33 = mat<3, 3>;
    using mat22 = mat<2, 2>;
}
