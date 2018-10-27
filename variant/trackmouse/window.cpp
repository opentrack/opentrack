/* Copyright (c) 2013-2018, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "window.hpp"
#include "options/options.hpp"
#include "migration/migration.hpp"
#include "compat/check-visible.hpp"
#include "compat/sleep.hpp"
#include "compat/macros.hpp"
#include "compat/library-path.hpp"
#include "compat/math.hpp"

#include <algorithm>
#include <iterator>
#include <utility>

#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QString>
#include <QList>
#include <QEventLoop>
#include <QApplication>

extern "C" const char* const opentrack_version;

using namespace options::globals;
using namespace options;

#if !defined EXIT_SUCCESS
#   define EXIT_SUCCESS 0
#endif

#if !defined EXIT_FAILURE
#   define EXIT_FAILURE 1
#endif

/* FreeBSD sysexits(3)
 *
 * The input data was incorrect	in some	way.  This
 * should only be used for user's data and not system
 * files.
 */

#if !defined EX_OSFILE
#   define EX_OSFILE 72
#endif

void force_trackmouse_settings();

main_window::main_window() : State(OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH)
{
    ui.setupUi(this);

    update_button_state(false, false);

    // ctrl+q exits
    connect(&kbd_quit, SIGNAL(activated()), this, SLOT(exit()));

    if (!set_profile())
    {
        die_on_config_not_writable();
        exit(EX_OSFILE);
        return;
    }

    // only tie and connect main screen options after migrations are done
    // below is fine, set_profile() is called already

    connect(this, &main_window::start_tracker,
            this, [&] { qDebug() << "start tracker"; start_tracker_(); },
            Qt::QueuedConnection);

    connect(this, &main_window::stop_tracker,
            this, [&] { qDebug() << "stop tracker"; stop_tracker_(); },
            Qt::QueuedConnection);

    connect(ui.btnStartTracker, SIGNAL(clicked()), this, SLOT(start_tracker_()));
    connect(ui.btnStopTracker, SIGNAL(clicked()), this, SLOT(stop_tracker_()));

    {
        tie_setting(mouse.sensitivity_x, ui.sensitivity_slider);
        tie_setting(mouse.sensitivity_x, ui.sensitivity_label,
                    [](double x) { return QString::number(x) + QStringLiteral("%"); });
        // one-way only
        tie_setting(mouse.sensitivity_x, this,
                    [this](double x) { mouse.sensitivity_y = *mouse.sensitivity_x; });

        // no "ok" button, gotta save on timer
        auto save = [this] {
            qDebug() << "trackmouse: saving settings";
            mouse.b->save();
            save_settings_timer.stop();
        };

        auto start_save_timer = [this](double) {
            save_settings_timer.start();
        };

        save_settings_timer.setInterval(save_settings_interval_ms);
        save_settings_timer.setSingleShot(true);

        ui.sensitivity_slider->setTracking(false);
        connect(&save_settings_timer, &QTimer::timeout, this, save, Qt::DirectConnection);
#if 1
        // this doesn't fire the timer on application load
        connect(ui.sensitivity_slider, &QSlider::valueChanged, this, start_save_timer, Qt::DirectConnection);
#else
        // but this does so let's not not use it
        tie_setting(mouse.sensitivity_x, this, start_save_timer);
#endif
    }

    force_trackmouse_settings();

    register_shortcuts();
    kbd_quit.setEnabled(true);

    setWindowFlags(Qt::MSWindowsFixedSizeDialogHint | windowFlags());
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    adjustSize();

    setVisible(true);
    show();
}

void main_window::register_shortcuts()
{
    global_shortcuts.reload({
        { s.key_toggle_tracking1, [this](bool) { main_window::toggle_tracker_(); }, true },
    });

    if (work)
        work->reload_shortcuts();
}

void main_window::die_on_config_not_writable()
{
    stop_tracker_();

    static const QString pad(16, QChar(' '));

    QMessageBox::critical(this,
                          tr("The Octopus is sad"),
                          tr("Check permissions for your .ini directory:\n\n%1\"%2\n\n"
                             "Exiting now."
                          ).arg(ini_directory(), pad),
                          QMessageBox::Close, QMessageBox::NoButton);

    exit(EX_OSFILE);
}

bool main_window::maybe_die_on_config_not_writable(const QString& current)
{
    const bool writable =
        with_settings_object([&](QSettings& s) {
            return s.isWritable();
        });

    if (writable)
        return false;

    if (!QFile(ini_combine(current)).open(QFile::ReadWrite))
    {
        die_on_config_not_writable();
        return true;
    }

    return false;
}

main_window::~main_window()
{
    // stupid ps3 eye has LED issues
    if (work)
    {
        stop_tracker_();
        QEventLoop ev;
        ev.processEvents();
        portable::sleep(2000);
    }

    exit();
}

void main_window::set_working_directory()
{
    QDir::setCurrent(OPENTRACK_BASE_PATH);
}

void main_window::save_modules()
{
    m.b->save();
}

std::tuple<main_window::dylib_ptr, int>
main_window::module_by_name(const QString& name, Modules::dylib_list& list)
{
    auto it = std::find_if(list.cbegin(), list.cend(), [&name](const dylib_ptr& lib) {
        if (!lib)
            return name.isEmpty();
        else
            return name == lib->module_name;
    });

    if (it == list.cend())
        return { nullptr, -1 };
    else
        return { *it, std::distance(list.cbegin(), it) };
}

main_window::dylib_ptr main_window::current_tracker()
{
    auto [ptr, idx] = module_by_name(m.tracker_dll, modules.trackers());
    return ptr;
}

main_window::dylib_ptr main_window::current_protocol()
{
    auto [ptr, idx] = module_by_name(m.protocol_dll, modules.protocols());
    return ptr;
}

main_window::dylib_ptr main_window::current_filter()
{
    auto [ptr, idx] = module_by_name(m.filter_dll, modules.filters());
    return ptr;
}

void main_window::update_button_state(bool running, bool inertialp)
{
    bool not_running = !running;
#if 0
    ui.iconcomboProfile->setEnabled(not_running);
    ui.btnStartTracker->setEnabled(not_running);
    ui.btnStopTracker->setEnabled(running);
    ui.iconcomboProtocol->setEnabled(not_running);
    ui.iconcomboFilter->setEnabled(not_running);
    ui.iconcomboTrackerSource->setEnabled(not_running);
    ui.profile_button->setEnabled(not_running);
#endif
    ui.video_frame_label->setVisible(not_running || inertialp);
    if(not_running)
    {
        ui.video_frame_label->setPixmap(QPixmap(":/images/tracking-not-started.png"));
    }
    else {
        ui.video_frame_label->setPixmap(QPixmap(":/images/no-feed.png"));
    }
}

void main_window::start_tracker_()
{
    if (work)
        return;

    work = std::make_shared<Work>(pose, ev, ui.video_frame, current_tracker(), current_protocol(), current_filter());

    if (!work->is_ok())
    {
        work = nullptr;
        return;
    }

    if (pTrackerDialog)
        pTrackerDialog->register_tracker(work->libs.pTracker.get());

    if (pFilterDialog)
        pFilterDialog->register_filter(work->libs.pFilter.get());

    if (pProtocolDialog)
        pProtocolDialog->register_protocol(work->libs.pProtocol.get());

    // NB check valid since SelectedLibraries ctor called
    // trackers take care of layout state updates
    const bool is_inertial = ui.video_frame->layout() == nullptr;
    update_button_state(true, is_inertial);

    ui.btnStopTracker->setFocus();
}

void main_window::stop_tracker_()
{
    if (!work)
        return;

    with_tracker_teardown sentinel;

    if (pTrackerDialog)
        pTrackerDialog->unregister_tracker();

    if (pProtocolDialog)
        pProtocolDialog->unregister_protocol();

    if (pFilterDialog)
        pFilterDialog->unregister_filter();

    work = nullptr;

    update_button_state(false, false);
    set_title();
    ui.btnStartTracker->setFocus();

    // ps3 eye issues
    portable::sleep(1000);
}

void main_window::set_title(const QString& game_title)
{
    static const QString version{opentrack_version};
    static const QString sep { tr(" :: ") };
    static const QString pat1{ version + sep + "%1" + sep + "%2" };
    static const QString pat2{ version + sep + "%1" };

    const QString current = ini_filename();

    if (game_title.isEmpty())
        setWindowTitle(pat2.arg(current));
    else
        setWindowTitle(pat1.arg(current, game_title));
}

void main_window::exit(int status)
{
    if (exiting_already)
        return;
    exiting_already = true;

    qDebug() << "trackmouse: saving settings on app exit";
    save_settings_timer.stop();
    mouse.b->save();

    //close();
    QApplication::setQuitOnLastWindowClosed(true);
    QApplication::exit(status);
}

bool main_window::set_profile()
{
    if (maybe_die_on_config_not_writable(OPENTRACK_DEFAULT_CONFIG))
        return false;

    set_profile_in_registry();

    options::detail::bundler::refresh_all_bundles();

    // migrations are for config layout changes and other user-visible
    // incompatibilities in future versions
    run_migrations();

    set_title();

    return true;
}

void main_window::closeEvent(QCloseEvent*)
{
    exit();
}

void main_window::set_profile_in_registry()
{
    with_global_settings_object([&](QSettings& s) {
        s.setValue(OPENTRACK_CONFIG_FILENAME_KEY, OPENTRACK_DEFAULT_CONFIG);
    });
}

void main_window::toggle_tracker_()
{
    if (work)
        stop_tracker();
    else
        start_tracker();
}
