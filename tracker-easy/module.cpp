#include "tracker-easy.h"
#include "tracker-easy-dialog.h"
#include "tracker-easy-api.h"
#include "module.hpp"
#include "cv-point-extractor.h"

#include <memory>



namespace EasyTracker
{

    QString Metadata::name() { return tr("Easy Tracker 0.1"); }
    QIcon Metadata::icon() { return QIcon(":/Resources/Logo_IR.png"); }

}


OPENTRACK_DECLARE_TRACKER(EasyTracker::Tracker, EasyTracker::Dialog, EasyTracker::Metadata)
