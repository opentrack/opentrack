#pragma once

#include "api/plugin-api.hpp"

class wii_metadata_pt : public Metadata
{
    Q_OBJECT

    QString name() override;
    QIcon icon() override;
};
