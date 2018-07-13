#include "plugin-api.hpp"

#include <utility>

using namespace plugin_api::detail;

// these exist so that vtable is emitted in a single compilation unit, not all of them.

Metadata_::~Metadata_() = default;
IFilter::~IFilter() = default;
IProtocol::~IProtocol() = default;
ITracker::~ITracker() = default;
IExtension::~IExtension() = default;

void ITrackerDialog::register_tracker(ITracker*) {}
void ITrackerDialog::unregister_tracker() {}

BaseDialog::BaseDialog() = default;

void BaseDialog::closeEvent(QCloseEvent*)
{
    if (isVisible())
    {
        hide();
        emit closing();
    }
}

bool ITracker::center() { return false; }

module_status ITracker::status_ok()
{
    return module_status();
}

module_status ITracker::error(const QString& error)
{
    return module_status(error);
}

Metadata_::Metadata_() = default;
IFilter::IFilter() = default;
IFilterDialog::IFilterDialog() = default;
IProtocol::IProtocol() = default;
IProtocolDialog::IProtocolDialog() = default;
ITracker::ITracker() = default;
ITrackerDialog::ITrackerDialog() = default;

void BaseDialog::done(int)
{
    if (isVisible())
    {
        hide();
        close();
    }
}

IExtensionDialog::~IExtensionDialog() = default;

bool module_status::is_ok() const
{
    return error.isNull();
}

module_status::module_status(QString error) : error(std::move(error)) {}

module_status module_status_mixin::status_ok() { return {}; }

module_status module_status_mixin::error(const QString& error)
{
    return module_status(error.isEmpty() ? "Unknown error" : error);
}


Metadata::Metadata() = default;
Metadata::~Metadata() = default;
