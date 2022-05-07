#include "state.hpp"
#include "opentrack/defs.hpp"
#include <iterator>

using dylib_ptr = Modules::dylib_ptr;
using dylib_list = Modules::dylib_list;

std::tuple<dylib_ptr, int> State::module_by_name(const QString& name, dylib_list& list)
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

State::State(const QString& library_path) :
    modules(library_path),
    pose(s.all_axis_opts),
    library_path{library_path}
{}

dylib_ptr State::current_tracker()
{
    const QString& module =
#ifdef UI_FORCED_TRACKER
        UI_FORCED_TRACKER;
#else
        m.tracker_dll;
#endif
    auto [ptr, idx] = module_by_name(module, modules.trackers());
    return ptr;
}

dylib_ptr State::current_protocol()
{
    auto [ptr, idx] = module_by_name(m.protocol_dll, modules.protocols());
    return ptr;
}

dylib_ptr State::current_filter()
{
    const QString& module =
#ifdef UI_FORCED_FILTER
        UI_FORCED_FILTER;
#else
        m.filter_dll;
#endif
    auto [ptr, idx] = module_by_name(module, modules.filters());
    return ptr;
}
