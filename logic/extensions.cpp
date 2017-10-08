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
    { &IExtension::process_before_filter, ext_mask::on_before_filter, ext_ord::ev_before_filter, },
    { &IExtension::process_before_mapping, ext_mask::on_before_mapping, ext_ord::ev_before_mapping, },
    { &IExtension::process_finished, ext_mask::on_finished, ext_ord::ev_finished, },
};

bool event_handler::is_enabled(const QString& name)
{
#if 1
    return true;
#else
    if (!ext_bundle->contains(name))
        return false;

    return ext_bundle->get<bool>(name);
#endif
}

event_handler::event_handler(Modules::dylib_list const& extensions) : ext_bundle(make_bundle("extensions"))
{
    for (std::shared_ptr<dylib> const& lib : extensions)
    {
        std::shared_ptr<IExtension> ext(reinterpret_cast<IExtension*>(lib->Constructor()));
        std::shared_ptr<IExtensionDialog> dlg(reinterpret_cast<IExtensionDialog*>(lib->Dialog()));
        std::shared_ptr<Metadata> m(reinterpret_cast<Metadata*>(lib->Meta()));

        const ext_mask mask = ext->hook_types();

        if (!is_enabled(lib->module_name))
            continue;

#if 1
        qDebug() << "extension" << lib->module_name << "mask" << (void*)mask;
#endif

        for (event_type_mapping const& mapping : ordinal_to_function)
        {
            const unsigned i = mapping.idx;
            const ext_mask mask_ = mapping.mask;

            if (mask & mask_)
                extensions_for_event[i].push_back({ ext, dlg, m });
        }
    }
}

void event_handler::run_events(event_ordinal k, Pose& pose)
{
    auto fun = std::mem_fn(ordinal_to_function[k].ptr);

    for (extension& x : extensions_for_event[k])
        fun(*x.logic, pose);
}

