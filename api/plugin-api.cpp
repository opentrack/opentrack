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
