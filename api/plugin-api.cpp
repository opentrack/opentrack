#include "plugin-api.hpp"

#include <utility>

namespace plugin_api::detail {

BaseDialog::BaseDialog() = default;
void BaseDialog::closeEvent(QCloseEvent*)
{
    if (isVisible())
    {
        hide();
        emit closing();
    }
}

void BaseDialog::done(int)
{
    if (isVisible())
    {
        hide();
        close();
    }
}

} // ns plugin_api::detail

// these exist so that vtable is emitted in a single compilation unit, not all of them.

Metadata_::Metadata_() = default;
Metadata_::~Metadata_() = default;
Metadata::Metadata() = default;
Metadata::~Metadata() = default;

IFilter::IFilter() = default;
IFilter::~IFilter() = default;
IFilterDialog::IFilterDialog() = default;
IFilterDialog::~IFilterDialog() = default;
IProtocol::IProtocol() = default;
IProtocol::~IProtocol() = default;
IProtocolDialog::IProtocolDialog() = default;
IProtocolDialog::~IProtocolDialog() = default;
ITracker::ITracker() = default;
ITracker::~ITracker() = default;
bool ITracker::center() { return false; }
ITrackerDialog::ITrackerDialog() = default;
ITrackerDialog::~ITrackerDialog() = default;
void ITrackerDialog::register_tracker(ITracker*) {}
void ITrackerDialog::unregister_tracker() {}
IExtension::~IExtension() = default;
IExtensionDialog::~IExtensionDialog() = default;

bool module_status::is_ok() const
{
    return error.isNull();
}

module_status_mixin::~module_status_mixin() = default;
module_status::module_status(QString error) : error(std::move(error)) {}
module_status module_status_mixin::status_ok() { return {}; }

module_status module_status_mixin::error(const QString& error)
{
    return module_status(error.isEmpty() ? "Unknown error" : error);
}

module_status ITracker::status_ok()
{
    return module_status();
}

module_status ITracker::error(const QString& error)
{
    return module_status(error);
}
