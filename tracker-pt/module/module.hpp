#pragma once

#include "api/plugin-api.hpp"
#include <QtGlobal>
#include <QIcon>

namespace pt_module
{

class Q_DECL_EXPORT metadata_pt : public Metadata
{
    Q_OBJECT

    QString name();
    QIcon icon();
};

} // ns pt_module
