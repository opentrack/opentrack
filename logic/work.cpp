#include "work.hpp"
#include "compat/library-path.hpp"

#include <utility>

#include <QObject>
#include <QMessageBox>
#include <QFileDialog>

QString Work::browse_datalogging_file(main_settings &s)
{
    QString filename = s.tracklogging_filename;
    if (filename.isEmpty())
        filename = OPENTRACK_BASE_PATH;
    /* Sometimes this function freezes the app before opening the dialog.
       Might be related to https://forum.qt.io/topic/49209/qfiledialog-getopenfilename-hangs-in-windows-when-using-the-native-dialog/8
       and be a known problem. Possible solution is to use the QFileDialog::DontUseNativeDialog flag.
       Since the freeze is apparently random, I'm not sure it helped.
    */
    QString newfilename = QFileDialog::getSaveFileName(nullptr,
                                                       tr("Select filename"),
                                                       filename,
                                                       tr("CSV File (*.csv)"),
                                                       nullptr);
    if (!newfilename.isEmpty())
    {
      s.tracklogging_filename = newfilename;
    }
    // dialog likes to mess with current directory
    QDir::setCurrent(OPENTRACK_BASE_PATH);
    return newfilename;
}

std::unique_ptr<TrackLogger> Work::make_logger(main_settings &s)
{
    if (s.tracklogging_enabled)
    {
        QString filename = browse_datalogging_file(s);
        if (filename.isEmpty())
        {
            // The user probably canceled the file dialog. In this case we don't want to do anything.
            return {};
        }
        else
        {
            auto logger = std::make_unique<TrackLoggerCSV>(s.tracklogging_filename);

            if (!logger->is_open())
            {
                QMessageBox::warning(nullptr,
                    tr("Logging error"),
                    tr("Unable to open file '%1'. Proceeding without logging.").arg(s.tracklogging_filename),
                    QMessageBox::Ok, QMessageBox::NoButton);
            }
            else
                return logger;
        }
    }

    return std::make_unique<TrackLogger>();
}


Work::Work(Mappings& m, event_handler& ev, QFrame* frame,
           const dylibptr& tracker_, const dylibptr& filter_, const dylibptr& proto_) :
    libs(frame, tracker_, filter_, proto_),
    logger{make_logger(s)},
    pipeline_{ m, libs, ev, *logger },
    keys {
#if defined OTR_HAS_KEY_UP_SUPPORT
        key_tuple(s.key_center1, [&](bool x) { pipeline_.set_center(); pipeline_.set_held_center(x); }, false),
        key_tuple(s.key_center2, [&](bool x) { pipeline_.set_center(); pipeline_.set_held_center(x); }, false),
#else
        key_tuple(s.key_center1, [&](bool) { pipeline_.set_center(); }, true),
        key_tuple(s.key_center2, [&](bool) { pipeline_.set_center(); }, true),
#endif

        key_tuple(s.key_toggle1, [&](bool) { pipeline_.toggle_enabled(); }, true),
        key_tuple(s.key_toggle2, [&](bool) { pipeline_.toggle_enabled(); }, true),

        key_tuple(s.key_zero1, [&](bool) { pipeline_.toggle_zero(); }, true),
        key_tuple(s.key_zero2, [&](bool) { pipeline_.toggle_zero(); }, true),

        key_tuple(s.key_toggle_press1, [&](bool x) { pipeline_.set_enabled(!x); }, false),
        key_tuple(s.key_toggle_press2, [&](bool x) { pipeline_.set_enabled(!x); }, false),

        key_tuple(s.key_zero_press1, [&](bool x) { pipeline_.set_zero(x); }, false),
        key_tuple(s.key_zero_press2, [&](bool x) { pipeline_.set_zero(x); }, false),
    }
{
    if (!is_ok())
        return;
    reload_shortcuts();
    pipeline_.start();
}

void Work::reload_shortcuts()
{
    sc.reload(keys);
}

bool Work::is_ok() const
{
    return libs.correct;
}

// TODO member dtor order looks fine, check valgrind -sh 20180706
#if 0
Work::~Work()
{
    // order matters, otherwise use-after-free -sh
    //sc = nullptr;
    //pipeline = nullptr;
    //libs = runtime_libraries();
}
#endif
