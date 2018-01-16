#pragma once

#include "options/options.hpp"

namespace mouse_impl {

using namespace options;

struct mouse_settings : opts {
    value<int> Mouse_X, Mouse_Y;
    value<slider_value> sensitivity_x, sensitivity_y;
    mouse_settings() :
        opts("mouse-proto"),
        Mouse_X(b, "mouse-x", 0),
        Mouse_Y(b, "mouse-y", 0),
        sensitivity_x(b, "mouse-sensitivity-x", slider_value(200, 25, 500)),
        sensitivity_y(b, "mouse-sensitivity-y", slider_value(200, 25, 500))
    {}
};

} // ns mouse_impl

using mouse_impl::mouse_settings;
