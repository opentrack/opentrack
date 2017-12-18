#pragma once

#include "api/plugin-api.hpp"
#include "compat/util.hpp"

struct falcon_bms_acceleration_ext : IExtension
{
    event_mask hook_types() override;
    falcon_bms_acceleration_ext();
    void process_finished(Pose&p) override;
    module_status initialize() override { return status_ok(); }
};
