#include "metadata.hpp"
#include "api/plugin-api.hpp"
#include "trackhat.hpp"
#include "dialog.hpp"

// XXX TODO
const QString trackhat_metadata::module_name = QStringLiteral("tracker-trackhat/pt");

pt_runtime_traits::pointer<pt_camera> trackhat_metadata::make_camera() const
{
    return std::make_shared<trackhat_camera>();
}

pt_runtime_traits::pointer<pt_point_extractor> trackhat_metadata::make_point_extractor() const
{
    return std::make_shared<trackhat_extractor>();
}

pt_runtime_traits::pointer<pt_frame> trackhat_metadata::make_frame() const
{
    return std::make_shared<trackhat_frame>();
}

pt_runtime_traits::pointer<pt_preview> trackhat_metadata::make_preview(int w, int h) const
{
    return std::make_shared<trackhat_preview>(w, h);
}

QString trackhat_metadata::get_module_name() const
{
    return trackhat_metadata::module_name;
}

trackhat_pt::trackhat_pt() :
      Tracker_PT(pt_runtime_traits::pointer<pt_runtime_traits>(new trackhat_metadata))
{
}

OPENTRACK_DECLARE_TRACKER(trackhat_pt, trackhat_dialog, trackhat_module)
