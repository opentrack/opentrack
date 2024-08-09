#if defined OTR_DBUS_CONTROL
#include "dbus.hpp"

bool DBus::is_enabled() const
{
    return pipeline_->is_enabled();
}

bool DBus::is_zero() const
{
    return pipeline_->is_zero();
}

void DBus::Center()
{
    pipeline_->set_center(true);
}

void DBus::ToggleEnabled()
{
    pipeline_->toggle_enabled();
}

void DBus::ToggleZero()
{
    pipeline_->toggle_zero();
}
#endif
