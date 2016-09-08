#include "work.hpp"
#include "opentrack-library-path.h"

#include <QMessageBox>
#include <QFileDialog>


static QString browse_datalogging_file(main_settings &s)
{
    QString filename = s.tracklogging_filename;
    if (filename.isEmpty())
        filename = OPENTRACK_BASE_PATH;
    /* Sometimes this function freezes the app before opening the dialog.
       Might be related to https://forum.qt.io/topic/49209/qfiledialog-getopenfilename-hangs-in-windows-when-using-the-native-dialog/8
       and be a known problem. Possible solution is to use the QFileDialog::DontUseNativeDialog flag.
       Since the freeze is apparently random, I'm not sure it helped.
    */
    QString newfilename = QFileDialog::getSaveFileName(nullptr, QFileDialog::tr("Select Filename"), filename, QFileDialog::tr("CSV File (*.csv)"), nullptr); //, QFileDialog::DontUseNativeDialog);
    if (!newfilename.isEmpty())
    {
      s.tracklogging_filename = newfilename;
    }
    // dialog likes to mess with current directory
    QDir::setCurrent(OPENTRACK_BASE_PATH);
    return newfilename;
}

std::shared_ptr<TrackLogger> Work::make_logger(main_settings &s)
{
    if (s.tracklogging_enabled)
    {
        QString filename = browse_datalogging_file(s);
        if (static_cast<QString>(s.tracklogging_filename).isEmpty())
        {
          // The user probably canceled the file dialog. In this case we don't want to do anything.
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
