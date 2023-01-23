#pragma once
#include "options/value.hpp"
using namespace options;

enum class exposure_preset : int {
    near, far, ignored,
    DEFAULT = near,
};

struct dshow_camera_settings final {
    bundle b = make_bundle("video-camera");
    value<exposure_preset> exposure_preset{b, "exposure-preset", exposure_preset::near};
};
