#pragma once

#include "api/plugin-api.hpp"

#include <QIcon>
#include <QtGlobal>

class Q_DECL_EXPORT wii_metadata_pt : public Metadata
{
    Q_OBJECT

    QString name();
    QIcon icon();
};
