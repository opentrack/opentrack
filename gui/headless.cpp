#include "headless.hpp"

#include "compat/appnap.hpp"
#include "compat/background-app.hpp"
#include "compat/base-path.hpp"
#include "compat/library-path.hpp"
#include "logic/state.hpp"
#include "logic/work.hpp"
#include "migration/migration.hpp"
#include "options/options.hpp"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QTimer>
#include <QtGlobal>

#ifdef Q_OS_UNIX
#   include <csignal>
#endif

#include <memory>

using namespace options;
using namespace options::globals;

static void copy_presets(const QString& library_path)
{
    // mirrors main_window::copy_presets()
    const QString preset_dir = library_path + "/presets/";
    const QDir dir{preset_dir};
    if (!dir.exists())
    {
        qDebug() << "no preset dir";
        return;
    }

    with_global_settings_object([&](QSettings& s) {
        const QString& key = QStringLiteral("last-preset-copy-time");
        const auto last_time = s.value(key, -1LL).toLongLong();
        for (const auto& file : dir.entryInfoList({ "*.ini" }, QDir::Files, QDir::Name))
        {
            if (file.fileName() == QStringLiteral("default.ini"))
                continue;
            if (last_time < file.lastModified().toSecsSinceEpoch())
            {
                qDebug() << "copy preset" << file.fileName();
                (void)QFile::copy(file.filePath(), ini_combine(file.fileName()));
            }
        }
        s.setValue(key, QDateTime::currentSecsSinceEpoch());
    });
}

static void set_profile_in_registry(const QString& profile)
{
    with_global_settings_object([&](QSettings& s) {
        s.setValue(QStringLiteral(OPENTRACK_PROFILE_FILENAME_KEY), profile);
        mark_global_ini_modified();
    });
}

#ifdef Q_OS_UNIX
static volatile sig_atomic_t headless_stop_requested = 0;

static void headless_signal_handler(int)
{
    headless_stop_requested = 1;
}
#endif

int run_headless(const QString& profile_arg)
{
    // logic/ modules check this to suppress blocking modal dialogs
    qputenv("OTR_HEADLESS", "1");

#ifdef __APPLE__
    // no Dock icon, no menu bar: a headless run is a background agent
    set_background_app();
    QTimer::singleShot(0, [] { set_background_app(); });
#endif

    const QString library_path = OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH;

    copy_presets(library_path);

    QString profile = profile_arg;
    if (!profile.isEmpty() && !ini_list().contains(profile))
    {
        // accept both "name" and "name.ini"
        if (ini_list().contains(profile + QStringLiteral(".ini")))
            profile += QStringLiteral(".ini");
        else
        {
            qWarning() << "opentrack: profile" << profile << "not found, using default";
            profile = QStringLiteral(OPENTRACK_DEFAULT_PROFILE);
        }
    }

    if (!profile.isEmpty())
        set_profile_in_registry(profile);

    // load the (possibly new) profile's settings before constructing any
    // options bundles, then apply config layout migrations
    options::detail::bundler::reload_no_notify();
    (void)run_migrations();
    options::detail::bundler::notify();

    State state(library_path);

    if (state.m.tracker_dll->isEmpty() || state.m.protocol_dll->isEmpty())
    {
        qCritical() << "opentrack: profile" << ini_filename()
                    << "has no tracker/protocol selected;"
                    << "open the GUI once and pick them, then retry";
        return 1;
    }

    // some trackers put camera preview widgets in here; keep it hidden
    QFrame frame;
    frame.resize(320, 240);

    // argument order matches main_window::start_tracker_()
    auto work = std::make_shared<Work>(state.pose, &frame,
                                       state.current_tracker(),
                                       state.current_protocol(),
                                       state.current_filter());
    if (!work->is_ok())
    {
        qCritical() << "opentrack: failed to start tracking";
        return 1;
    }

#ifdef __APPLE__
    disable_appnap_start();
#endif

    qInfo() << "opentrack: headless tracking started,"
            << "profile" << ini_filename()
            << "tracker" << state.m.tracker_dll
            << "protocol" << state.m.protocol_dll
            << "filter" << state.m.filter_dll;

#ifdef Q_OS_UNIX
    struct sigaction sa {};
    sa.sa_handler = headless_signal_handler;
    (void)sigaction(SIGINT, &sa, nullptr);
    (void)sigaction(SIGTERM, &sa, nullptr);

    QTimer stop_poller;
    stop_poller.setInterval(200);
    QObject::connect(&stop_poller, &QTimer::timeout, [&] {
        if (headless_stop_requested)
            QApplication::quit();
    });
    stop_poller.start();
#endif

    const int ret = QApplication::exec();

#ifdef __APPLE__
    disable_appnap_stop();
#endif

    // `work` must outlive the event loop; destroying it stops the pipeline
    return ret;
}
