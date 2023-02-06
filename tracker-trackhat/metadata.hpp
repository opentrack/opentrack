#pragma once
#include "trackhat.hpp"
#include "../tracker-pt/ftnoir_tracker_pt.h"

class trackhat_pt final : public Tracker_PT
{
    Q_OBJECT

public:
    trackhat_pt();
};

class trackhat_module final : public Metadata
{
    Q_OBJECT

public:
    QString name() override { return tr("TrackHat v1 Sensor"); }
    QIcon icon() override { return QIcon(":/images/trackhat-64x64.png"); }
    static const QString module_name;
};
