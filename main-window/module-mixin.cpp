#include "module-mixin.hpp"

#include <algorithm>

namespace OTR_MIXIN_NS(module_mixin) {

std::tuple<dylib_ptr, int>
module_mixin::module_by_name(const QString& name, const dylib_list& list) const
{
    auto it = std::find_if(list.cbegin(), list.cend(), [&name](const dylib_ptr& lib) {
        if (!lib)
            return name.isEmpty();
        else
            return name == lib->module_name;
    });

    if (it == list.cend())
        return { nullptr, -1 };
    else
        return { *it, int(std::distance(list.cbegin(), it)) };
}

static void show_window(QWidget& d, bool fresh)
{
    if (fresh)
    {
        d.setWindowFlags(Qt::MSWindowsFixedSizeDialogHint | d.windowFlags());
        d.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        d.show();
        d.adjustSize();
        d.raise();
    }
    else
    {
        d.show();
        d.raise();
    }
}

template<typename t, typename F>
static bool mk_window_common(std::unique_ptr<t>& d, F&& fun)
{
    bool fresh = false;

    if (!d)
        d = fun(), fresh = !!d;

    if (d)
        show_window(*d, fresh);

    return fresh;
}

template<typename t>
static bool mk_dialog(std::unique_ptr<t>& place, const std::shared_ptr<dylib>& lib)
{
    using u = std::unique_ptr<t>;

    return mk_window_common(place, [&] {
        if (lib && lib->Dialog)
            return u{ (t*)lib->Dialog() };
        else
            return nullptr;
    });
}

dylib_ptr module_mixin::current_tracker()
{
    auto [ptr, idx] = module_by_name(s.tracker_dll, modules.trackers());
    return ptr;
}

dylib_ptr module_mixin::current_protocol()
{
    auto [ptr, idx] = module_by_name(s.protocol_dll, modules.protocols());
    return ptr;
}

dylib_ptr module_mixin::current_filter()
{
    auto [ptr, idx] = module_by_name(s.filter_dll, modules.filters());
    return ptr;
}

void module_mixin::show_tracker_settings()
{
#if 0
    if (mk_dialog(tracker_dialog, current_tracker()) && work && work->libs.pTracker)
        tracker_dialog->register_tracker(work->libs.pTracker.get());
    if (tracker_dialog)
        QObject::connect(tracker_dialog.get(), &ITrackerDialog::closing,
                         this, [this] { tracker_dialog = nullptr; });
#endif
}

void module_mixin::show_proto_settings()
{
#if 0
    if (mk_dialog(proto_dialog, current_protocol()) && work && work->libs.pProtocol)
        proto_dialog->register_protocol(work->libs.pProtocol.get());
    if (proto_dialog)
        QObject::connect(proto_dialog.get(), &IProtocolDialog::closing,
                         this, [this] { proto_dialog = nullptr; });
#endif
}

void module_mixin::show_filter_settings()
{
#if 0
    if (mk_dialog(filter_dialog, current_filter()) && work && work->libs.pFilter)
        filter_dialog->register_filter(work->libs.pFilter.get());
    if (filter_dialog)
        QObject::connect(filter_dialog.get(), &IFilterDialog::closing,
                         this, [this] { filter_dialog = nullptr; });
#endif
}

template<typename t, typename... Args>
static bool mk_window(std::unique_ptr<t>& place, Args&&... params)
{
    return mk_window_common(place, [&] {
        return std::make_unique<t>(params...);
    });
}

module_mixin::module_mixin() = default;
module_mixin::~module_mixin() = default;

module_settings::module_settings() = default;

} // ns
