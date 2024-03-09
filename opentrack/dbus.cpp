#if defined OTR_DBUS_CONTROL
#include "dbus.hpp"
#include "main-window.hpp"

bool MainDBus::tracker_running() const
{
    return window_->tracker_running();
}

void MainDBus::StartTracker()
{
    window_->start_tracker_();
}

void MainDBus::StopTracker()
{
    window_->stop_tracker_();
}

void MainDBus::RestartTracker()
{
    window_->restart_tracker_();
}

void MainDBus::ToggleTracker()
{
    window_->toggle_tracker_();
}
#endif
