#pragma once
#include "api/plugin-api.hpp"

class osc_metadata : public Metadata
{
    Q_OBJECT

    QString name() override;
    QIcon icon() override;
};
