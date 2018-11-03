#pragma once

#include "mixins.hpp"
#include "compat/library-path.hpp"
#include "api/plugin-api.hpp"
#include "logic/extensions.hpp"
#include "logic/state.hpp"
#include "logic/main-settings.hpp"

#include <memory>
#include <utility>

#include <QObject>

namespace OTR_MIXIN_NS(module_mixin) {

using namespace options;

using dylib_ptr = Modules::dylib_ptr;
using dylib_list = Modules::dylib_list;

struct OTR_MAIN_EXPORT module_mixin
{
    module_mixin();
    virtual ~module_mixin();

    std::unique_ptr<ITrackerDialog> tracker_dialog;
    std::unique_ptr<IProtocolDialog> proto_dialog;
    std::unique_ptr<IFilterDialog> filter_dialog;

    std::tuple<dylib_ptr, int> module_by_name(const QString& name, const dylib_list& list) const;

    dylib_ptr current_tracker();
    dylib_ptr current_protocol();
    dylib_ptr current_filter();

    void show_tracker_settings_();
    void show_proto_settings_();
    void show_filter_settings_();

private:
    Modules modules { OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH };
    event_handler ev { modules.extensions() };
    module_settings s;
    State state { OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH };

    QObject fuzz;
};

}

OTR_DECLARE_MIXIN(module_mixin)
