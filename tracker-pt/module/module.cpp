#include "ftnoir_tracker_pt.h"

#include "module.hpp"
#include "camera.h"
#include "frame.hpp"
#include "point_extractor.h"
#include "ftnoir_tracker_pt_dialog.h"

#include "pt-api.hpp"

#include <memory>

static const QString module_name = "tracker-pt";

#ifdef __clang__
#   pragma clang diagnostic ignored "-Wweak-vtables"
#endif

namespace pt_module {

struct pt_module_traits final : pt_runtime_traits
{
    pointer<pt_camera> make_camera() const override
    {
        return pointer<pt_camera>(new Camera(module_name));
    }

    pointer<pt_point_extractor> make_point_extractor() const override
    {
        return pointer<pt_point_extractor>(new PointExtractor(module_name));
    }

    QString get_module_name() const override
    {
        return module_name;
    }

    pointer<pt_frame> make_frame() const override
    {
        return pointer<pt_frame>(new Frame);
    }

    pointer<pt_preview> make_preview(int w, int h) const override
    {
        return pointer<pt_preview>(new Preview(w, h));
    }
};

struct tracker_pt : Tracker_PT
{
    tracker_pt() : Tracker_PT(pointer<pt_runtime_traits>(new pt_module_traits))
    {
    }
};

struct dialog_pt : TrackerDialog_PT
{
    dialog_pt();
};

dialog_pt::dialog_pt() : TrackerDialog_PT(module_name) {}

QString metadata_pt::name() { return tr("PointTracker 1.1"); }
QIcon metadata_pt::icon() { return QIcon(":/Resources/Logo_IR.png"); }

}

// ns pt_module

using namespace pt_module;

OPENTRACK_DECLARE_TRACKER(tracker_pt, dialog_pt, metadata_pt)
