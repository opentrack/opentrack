#include "tracker-easy.h"
#include "tracker-easy-dialog.h"
#include "tracker-easy-api.h"

#include "module.hpp"
#include "frame.hpp"
#include "point_extractor.h"
#include "cv-point-extractor.h"


#include <memory>

static const QString module_name = "tracker-easy";

#ifdef __clang__
#   pragma clang diagnostic ignored "-Wweak-vtables"
#endif

namespace pt_module {

struct pt_module_traits final : pt_runtime_traits
{
    pointer<pt_point_extractor> make_point_extractor() const override
    {
        return pointer<pt_point_extractor>(new CvPointExtractor(module_name));
    }

    QString get_module_name() const override
    {
        return module_name;
    }

};

struct tracker_pt : EasyTracker
{
    tracker_pt() : EasyTracker(pointer<pt_runtime_traits>(new pt_module_traits))
    {
    }
};

struct dialog_pt : EasyTrackerDialog
{
    dialog_pt();
};

dialog_pt::dialog_pt() : EasyTrackerDialog(module_name) {}

QString metadata_pt::name() { return tr("Easy Tracker 0.1"); }
QIcon metadata_pt::icon() { return QIcon(":/Resources/Logo_IR.png"); }

}

// ns pt_module

using namespace pt_module;

OPENTRACK_DECLARE_TRACKER(tracker_pt, dialog_pt, metadata_pt)
