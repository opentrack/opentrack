#pragma once

#include "api/plugin-support.hpp"
#include "options/options.hpp"

#include <vector>
#include <array>

#include "export.hpp"

struct OTR_LOGIC_EXPORT event_handler final
{
    using event_ordinal = IExtension::event_ordinal;

    struct extension
    {
        using ext = std::shared_ptr<IExtension>;
        using dlg = std::shared_ptr<IExtensionDialog>;
        using m = std::shared_ptr<Metadata>;

        ext logic;
        dlg dialog;
        m metadata;
    };

    void run_events(event_ordinal k, Pose& pose);
    event_handler(Modules::dylib_list const& extensions);

private:
    using ext_list = std::vector<extension>;
    std::array<ext_list, IExtension::event_count> extensions_for_event;

    options::bundle ext_bundle;

    bool is_enabled(const QString& name);
};

