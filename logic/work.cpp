#include "work.hpp"

#include <QMessageBox>


std::shared_ptr<TrackLogger> Work::make_logger(const main_settings &s)
{
    if (s.tracklogging_enabled)
    {
        if (static_cast<QString>(s.tracklogging_filename).isEmpty())
        {
            QMessageBox::warning(nullptr, "Logging Error",
                "No filename given for track logging. Proceeding without logging.",
                QMessageBox::Ok,
                QMessageBox::NoButton);
        }
        else
        {
            auto logger = std::make_shared<TrackLoggerCSV>(s.tracklogging_filename);
            if (!logger->is_open())
            {
                logger = nullptr;
                QMessageBox::warning(nullptr, "Logging Error",
                    "Unable to open file: " + s.tracklogging_filename + ". Proceeding  without logging.",
                    QMessageBox::Ok,
                    QMessageBox::NoButton);
            }
            else
            {
                /* As this function has the potential to fill up the hard drive
                   of the unwary with junk data, a warning is in order. */
                QMessageBox::warning(nullptr, "Logging Active",
                    "Just a heads up. You are recoding pose data to " + s.tracklogging_filename + "!",
                    QMessageBox::Ok,
                    QMessageBox::NoButton);
                return logger;
            }
        }
    }
    return std::make_shared<TrackLogger>();
}


Work::Work(Mappings& m, SelectedLibraries& libs, WId handle) :
    libs(libs),
    logger(make_logger(s)),
    tracker(std::make_shared<Tracker>(m, libs, *logger)),
    sc(std::make_shared<Shortcuts>()),
    handle(handle),
    keys {
        key_tuple(s.key_center, [&](bool) -> void { tracker->center(); }, true),
        key_tuple(s.key_toggle, [&](bool) -> void { tracker->toggle_enabled(); }, true),
        key_tuple(s.key_zero, [&](bool) -> void { tracker->zero(); }, true),
        key_tuple(s.key_toggle_press, [&](bool x) -> void { tracker->set_toggle(!x); }, false),
        key_tuple(s.key_zero_press, [&](bool x) -> void { tracker->set_zero(x); }, false),
        key_tuple(s.key_disable_tcomp_press, [&](bool x) { tracker->set_tcomp_disabled(x); }, false),
    }
{
    reload_shortcuts();
    tracker->start();
}

void Work::reload_shortcuts()
{
    sc->reload(keys);
}

Work::~Work()
{
    // order matters, otherwise use-after-free -sh
    sc = nullptr;
    tracker = nullptr;
    libs = SelectedLibraries();
}
