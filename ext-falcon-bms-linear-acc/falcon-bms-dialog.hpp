#pragma once

#include "falcon-bms-ext.hpp"

class falcon_bms_acceleration_dialog : public IExtensionDialog
{
    Q_OBJECT

    void register_extension(IExtension& ext) override;
    void unregister_extension() override;
};
