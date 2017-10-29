#pragma once

#include "falcon-bms-ext.hpp"

struct falcon_bms_acceleration_dialog : IExtensionDialog
{
public:
    void register_extension(IExtension& ext) override;
    void unregister_extension() override;
};
