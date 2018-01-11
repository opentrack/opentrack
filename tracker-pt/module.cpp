#include "ftnoir_tracker_pt.h"
#include "api/plugin-api.hpp"

#include "camera.h"
#include "point_extractor.h"
#include "ftnoir_tracker_pt_dialog.h"

#include "pt-api.hpp"

#include <memory>

static const QString module_name = "tracker-pt";

struct pt_module_traits final : pt_runtime_traits
{
    std::unique_ptr<pt_camera> make_camera() const override
    {
        return std::unique_ptr<pt_camera>(new Camera);
    }

    std::unique_ptr<pt_point_extractor> make_point_extractor() const override
    {
        return std::unique_ptr<pt_point_extractor>(new PointExtractor(module_name));
    }

    QString get_module_name() const override
    {
        return module_name;
    }
};

struct pt_tracker_module : Tracker_PT
{
    pt_tracker_module() : Tracker_PT(pt_module_traits())
    {
    }
};

struct pt_tracker_dialog_module : TrackerDialog_PT
{
    pt_tracker_dialog_module() : TrackerDialog_PT(module_name) {}
};

class pt_module_metadata : public Metadata
{
    QString name() { return _("PointTracker 1.1"); }
    QIcon icon() { return QIcon(":/Resources/Logo_IR.png"); }
};

OPENTRACK_DECLARE_TRACKER(pt_tracker_module, pt_tracker_dialog_module, pt_module_metadata)
