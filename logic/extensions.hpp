#pragma once

#include "api/plugin-support.hpp"
#include "options/options.hpp"

#include <vector>
#include <array>

struct event_handler final
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
    std::array<ext_list, IExtension::event_count> extension_events;
};

struct ext_settings final
{
    static bool is_enabled(const QString& name);
    ext_settings() = delete;
};
