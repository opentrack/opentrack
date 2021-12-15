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

//static
void show_window(QWidget& d, bool fresh)
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
            return u{};
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

void module_mixin::show_tracker_settings_()
{
    if (mk_dialog(tracker_dialog, current_tracker()) && state.work && state.work->libs.pTracker)
        tracker_dialog->register_tracker(&*state.work->libs.pTracker);
    if (tracker_dialog)
        QObject::connect(&*tracker_dialog, &ITrackerDialog::closing,
                         &fuzz, [this] { tracker_dialog = nullptr; });
}

void module_mixin::show_proto_settings_()
{
    if (mk_dialog(proto_dialog, current_protocol()) && state.work && state.work->libs.pProtocol)
        proto_dialog->register_protocol(&*state.work->libs.pProtocol);
    if (proto_dialog)
        QObject::connect(&*proto_dialog, &IProtocolDialog::closing,
                         &fuzz, [this] { proto_dialog = nullptr; });
}

void module_mixin::show_filter_settings_()
{
    if (mk_dialog(filter_dialog, current_filter()) && state.work && state.work->libs.pFilter)
        filter_dialog->register_filter(&*state.work->libs.pFilter);
    if (filter_dialog)
        QObject::connect(&*filter_dialog, &IFilterDialog::closing,
                         &fuzz, [this] { filter_dialog = nullptr; });
}

// this template function must go to a separate function like "options_mixin".
template<typename t, typename... Args>
static bool mk_window(std::unique_ptr<t>& place, Args&&... params)
{
    return mk_window_common(place, [&] {
        return std::make_unique<t>(params...);
    });
}

module_mixin::module_mixin() = default;
module_mixin::~module_mixin() = default;

} // ns
