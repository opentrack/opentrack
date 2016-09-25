#include "plugin-api.hpp"

// these exist so that vtable is emitted in a single compilation unit, not all of them.

Metadata::~Metadata() {}
IFilter::~IFilter() {}
IFilterDialog::~IFilterDialog() {}
IProtocol::~IProtocol() {}
IProtocolDialog::~IProtocolDialog() {}
ITracker::~ITracker() {}
ITrackerDialog::~ITrackerDialog() {}

plugin_api::detail::BaseDialog::BaseDialog() {}
void plugin_api::detail::BaseDialog::closeEvent(QCloseEvent*) { emit closing(); }
Metadata::Metadata() {}
IFilter::IFilter() {}
IFilterDialog::IFilterDialog() {}
IProtocol::IProtocol() {}
IProtocolDialog::IProtocolDialog() {}
ITracker::ITracker() {}
ITrackerDialog::ITrackerDialog() {}
