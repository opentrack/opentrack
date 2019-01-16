#pragma once

#include "options/options.hpp"

enum input_method {
    input_direct = 0, input_legacy = 1,
};

namespace mouse_impl {

using namespace options;

struct mouse_settings : opts
{
    value<int> mouse_x { b, "mouse-x", 0 }, mouse_y { b, "mouse-y", 0 };
    value<slider_value> sensitivity_x { b, "mouse-sensitivity-x", { 200, 25, 500 } };
    value<slider_value> sensitivity_y { b, "mouse-sensitivity-y", { 200, 25, 500 } };
    value<input_method> input_method { b, "input-method", input_direct };

    mouse_settings() : opts("mouse-proto") {}
};

} // ns mouse_impl

using mouse_settings = mouse_impl::mouse_settings;
