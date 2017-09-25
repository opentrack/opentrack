#include "selected-libraries.hpp"
#include "options/scoped.hpp"
#include <QDebug>

using ext_ord = IExtension::event_ordinal;
using ext_mask = IExtension::event_mask;
using ext_fun_type = void(IExtension::*)(Pose&);

static constexpr struct event_type_mapping
{
    ext_fun_type ptr;
    ext_mask m;
    ext_ord idx;
} ordinal_to_function[] = {
    { &IExtension::process_raw, ext_mask::on_raw, ext_ord::ev_raw, },
    { &IExtension::process_after_center, ext_mask::on_after_center, ext_ord::ev_after_center, },
    { &IExtension::process_before_filter, ext_mask::on_before_filter, ext_ord::ev_before_filter, },
    { &IExtension::process_before_transform, ext_mask::on_before_transform, ext_ord::ev_before_transform, },
    { &IExtension::process_before_mapping, ext_mask::on_before_mapping, ext_ord::ev_before_mapping, },
    { &IExtension::process_finished, ext_mask::on_finished, ext_ord::ev_finished, },
};

SelectedLibraries::SelectedLibraries(QFrame* frame, dylibptr t, dylibptr p, dylibptr f) :
    pTracker(nullptr),
    pFilter(nullptr),
    pProtocol(nullptr),
    correct(false)
{
    using namespace options;

    const bool prev_teardown_flag = opts::is_tracker_teardown();

    opts::set_teardown_flag(true);

    pProtocol = make_dylib_instance<IProtocol>(p);

    if (!pProtocol)
    {
        qDebug() << "protocol dylib load failure";
        goto end;
    }

    if(!pProtocol->correct())
    {
        qDebug() << "protocol load failure";
        pProtocol = nullptr;
        goto end;
    }

    pTracker = make_dylib_instance<ITracker>(t);
    pFilter = make_dylib_instance<IFilter>(f);

    if (!pTracker)
    {
        qDebug() << "tracker dylib load failure";
        goto end;
    }

    pTracker->start_tracker(frame);

    correct = true;
end:
    opts::set_teardown_flag(prev_teardown_flag);
}

void runtime_event_handler::run_events(ext_event_ordinal k, Pose& pose)
{
    auto fun = std::mem_fn(ordinal_to_function[k].ptr);

    for (ext& x : extension_events[k])
    {
        if (x == nullptr)
            break;
        fun(x, pose);
    }
}
