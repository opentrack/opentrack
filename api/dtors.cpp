#include "plugin-api.hpp"

// these exist only so that vtable is emitted in a single compilation unit, not all of them.

Metadata::~Metadata() {}
IFilter::~IFilter() {}
IFilterDialog::~IFilterDialog() {}
IProtocol::~IProtocol() {}
IProtocolDialog::~IProtocolDialog() {}
ITracker::~ITracker() {}
ITrackerDialog::~ITrackerDialog() {}
