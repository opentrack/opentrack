#include "work.hpp"
#include "compat/library-path.hpp"

#include <qglobal.h>
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

        if (!filename.isEmpty())
        {
            auto logger = std::make_unique<TrackLoggerCSV>(*s.tracklogging_filename);

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


Work::Work(const Mappings& m, QFrame* frame,
           const dylibptr& tracker, const dylibptr& filter, const dylibptr& proto)
    : libs(frame, tracker, filter, proto)
    , pipeline_{ m, libs, *logger }
#if defined OTR_DBUS_CONTROL
    , dbus_(&dbus_obj, &pipeline_)
#endif
{
    if (!is_ok())
        return;
    reload_shortcuts();
    connect_dbus();
    pipeline_.start();
}

void Work::reload_shortcuts()
{
    sc.reload(keys);
}

void Work::connect_dbus()
{
#if defined OTR_DBUS_CONTROL
    qDebug() << "dbus: attaching work service";
    QDBusConnection::sessionBus().registerObject(DBus::SERVICE_PATH, &dbus_obj);
    if (!QDBusConnection::sessionBus().registerService(DBus::SERVICE_NAME))
    {
        qDebug() << "dbus: " << QDBusConnection::sessionBus().lastError().message();
    }
#endif
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
