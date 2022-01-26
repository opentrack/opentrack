#include "plugin-api.hpp"
#include <QCloseEvent>
#include <QDebug>

namespace plugin_api::detail {

BaseDialog::BaseDialog() = default;
void BaseDialog::closeEvent(QCloseEvent* e)
{
    if (isVisible())
        emit closing();
    e->accept();
}

void BaseDialog::done(int)
{
    if (isVisible())
        close();
}

bool BaseDialog::embeddable() noexcept { return false; }
void BaseDialog::save() {}
void BaseDialog::reload() {}
void BaseDialog::set_buttons_visible(bool x) {}

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
void IFilterDialog::register_filter(IFilter* filter) {}
void IFilterDialog::unregister_filter() {}
IProtocol::IProtocol() = default;
IProtocol::~IProtocol() = default;
IProtocolDialog::IProtocolDialog() = default;
IProtocolDialog::~IProtocolDialog() = default;
void IProtocolDialog::register_protocol(IProtocol* protocol){}
void IProtocolDialog::unregister_protocol() {}
ITracker::ITracker() = default;
ITracker::~ITracker() = default;
bool ITracker::center() { return false; }
ITrackerDialog::ITrackerDialog() = default;
ITrackerDialog::~ITrackerDialog() = default;
void ITrackerDialog::register_tracker(ITracker*) {}
void ITrackerDialog::unregister_tracker() {}

bool module_status::is_ok() const
{
    return error.isNull();
}
module_status_mixin::~module_status_mixin() = default;
module_status::module_status(const QString& error) : error(error) {}
module_status::module_status() = default;
module_status module_status_mixin::status_ok() { return {}; }

module_status module_status_mixin::error(const QString& error)
{
    return module_status(error.isEmpty() ? tr("Unknown error") : error);
}

module_status ITracker::status_ok()
{
    return module_status();
}

module_status ITracker::error(const QString& error)
{
    return module_status_mixin::error(error);
}
