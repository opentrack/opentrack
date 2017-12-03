#include "plugin-api.hpp"

using namespace plugin_api::detail;

// these exist so that vtable is emitted in a single compilation unit, not all of them.

Metadata::~Metadata() {}
IFilter::~IFilter() {}
IProtocol::~IProtocol() {}
ITracker::~ITracker() {}
IExtension::~IExtension() {}

void ITrackerDialog::register_tracker(ITracker*) {}
void ITrackerDialog::unregister_tracker() {}

BaseDialog::BaseDialog() {}

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

Metadata::Metadata() {}
IFilter::IFilter() {}
IFilterDialog::IFilterDialog() {}
IProtocol::IProtocol() {}
IProtocolDialog::IProtocolDialog() {}
ITracker::ITracker() {}
ITrackerDialog::ITrackerDialog() {}

void BaseDialog::done(int)
{
    if (isVisible())
    {
        hide();
        close();
    }
}

IExtensionDialog::~IExtensionDialog()
{
}

bool module_status::is_ok() const
{
    return error.isEmpty();
}

module_status::module_status(const QString& error) : error(error) {}

module_status module_status_mixin::status_ok() { return module_status(); }

module_status module_status_mixin::error(const QString& error) { return module_status(error); }
