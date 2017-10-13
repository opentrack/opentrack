#pragma once

#include "falcon-bms-ext.hpp"

struct falcon_bms_acceleration_metadata : Metadata
{
    QString name() override;
    QIcon icon() override;
};
