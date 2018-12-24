#pragma once

#include "api/plugin-api.hpp"
#include <QIcon>
#include <QString>

#include "compat/linkage-macros.hpp"

namespace pt_module
{

class OTR_GENERIC_EXPORT metadata_pt : public Metadata
{
    Q_OBJECT

    QString name() override;
    QIcon icon() override;
};

} // ns pt_module
