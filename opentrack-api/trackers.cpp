#include "opentrack-guts.h"
#include "opentrack.h"

extern "C" {

opentrack_tracker OPENTRACK_EXPORT opentrack_make_tracker(opentrack ctx, const char* name)
{
    for (int i = 0; i < ctx->meta_list.size(); i++)
    {
        auto meta = ctx->meta_list.at(i);
        if (ctx->meta_list.at(i).path == name)
        {
            ITracker* foo = static_cast<ITracker*>(meta.lib->Constructor());
            return foo;
        }
    }
    return NULL;
}

void OPENTRACK_EXPORT opentrack_finalize_tracker(opentrack_tracker tracker)
{
    delete tracker;
}

void OPENTRACK_EXPORT opentrack_tracker_start(opentrack self, opentrack_tracker tracker)
{
    // hot damn, this is problematic!
    // need to pass QFrame from somewhere
    return tracker->StartTracker(&self->fake_frame);
}

void OPENTRACK_EXPORT opentrack_tracker_tick(opentrack_tracker tracker, double* headpose)
{
    tracker->GetHeadPoseData(headpose);
    QApplication::processEvents(0, 5);
}

}
