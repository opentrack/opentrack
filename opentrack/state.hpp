#pragma once

#include <vector>
#include "opentrack/options.hpp"
using namespace options;
#include "opentrack/plugin-support.hpp"
#include "opentrack/main-settings.hpp"
#include "opentrack/mappings.hpp"
#include "opentrack/selected-libraries.hpp"
#include "opentrack/work.hpp"

struct State {
    State() :
        pose(std::vector<axis_opts*>{&s.a_x, &s.a_y, &s.a_z, &s.a_yaw, &s.a_pitch, &s.a_roll})
    {}
    Modules modules;
    SelectedLibraries libs;
    main_settings s;
    Mappings pose;
    mem<Work> work;
};
