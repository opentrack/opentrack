#include "state.hpp"

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
    ev(modules.extensions()),
    pose(s.all_axis_opts)
{}

dylib_ptr State::current_tracker()
{
    auto [ptr, idx] = module_by_name(m.tracker_dll, modules.trackers());
    return ptr;
}

dylib_ptr State::current_protocol()
{
    auto [ptr, idx] = module_by_name(m.protocol_dll, modules.protocols());
    return ptr;
}

dylib_ptr State::current_filter()
{
    auto [ptr, idx] = module_by_name(m.filter_dll, modules.filters());
    return ptr;
}
