#include "extensions.hpp"

#include <functional>
#include "compat/util.hpp"

using namespace options;

using ext_fun_type = void(IExtension::*)(Pose&);
using ext_mask = IExtension::event_mask;
using ext_ord = IExtension::event_ordinal;

static constexpr struct event_type_mapping
{
    ext_fun_type ptr;
    ext_mask mask;
    ext_ord idx;
} ordinal_to_function[] = {
    { &IExtension::process_raw, ext_mask::on_raw, ext_ord::ev_raw, },
    { &IExtension::process_after_center, ext_mask::on_after_center, ext_ord::ev_after_center, },
    { &IExtension::process_before_filter, ext_mask::on_before_filter, ext_ord::ev_before_filter, },
    { &IExtension::process_before_transform, ext_mask::on_before_transform, ext_ord::ev_before_transform, },
    { &IExtension::process_before_mapping, ext_mask::on_before_mapping, ext_ord::ev_before_mapping, },
    { &IExtension::process_finished, ext_mask::on_finished, ext_ord::ev_finished, },
};

bool ext_settings::is_enabled(const QString& name)
{
    static const bundle b = make_bundle("extensions");

    if (!b->contains(name))
        return false;

    return b->get<bool>(name);
}

event_handler::event_handler(Modules::dylib_list const& extensions)
{
    for (std::shared_ptr<dylib> const& lib : extensions)
    {
        std::shared_ptr<IExtension> ext(reinterpret_cast<IExtension*>(lib->Constructor()));
        std::shared_ptr<IExtensionDialog> dlg(reinterpret_cast<IExtensionDialog*>(lib->Dialog()));
        std::shared_ptr<Metadata> m(reinterpret_cast<Metadata*>(lib->Meta()));

        const ext_mask mask = ext->hook_types();

        if (!ext_settings::is_enabled(lib->module_name))
            continue;

        for (event_type_mapping const& mapping : ordinal_to_function)
        {
            const unsigned i = mapping.idx;
            const ext_mask mask_ = mapping.mask;

            if (mask & mask_)
                extension_events[i].push_back({ ext, dlg, m });
        }
    }
}

void event_handler::run_events(event_ordinal k, Pose& pose)
{
    auto fun = std::mem_fn(ordinal_to_function[k].ptr);

    for (extension& x : extension_events[k])
        fun(*x.logic, pose);
}

