#include "tracker-easy.h"
#include "tracker-easy-dialog.h"
#include "module.hpp"

#include <memory>

namespace EasyTracker
{
    QString Metadata::name() { return tr("Easy Tracker 1.1"); }
    QIcon Metadata::icon() { return QIcon(":/Resources/easy-tracker-logo.png"); }

}


OPENTRACK_DECLARE_TRACKER(EasyTracker::Tracker, EasyTracker::Dialog, EasyTracker::Metadata)
